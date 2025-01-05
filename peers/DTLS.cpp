#define _CRT_SECURE_NO_WARNINGS

#include "DTLS.h"
#include "Communicator.h" 
#include "Deserializer.h"

#define PROTOCOL_VERSION "Tslil and Yair DTLS"
#define CLIENT_PARENT 1
#define SERVER_CHILD 0

#ifndef MAC_SIZE
#define MAC_SIZE 32
#endif

std::vector<uint8_t> DTLS::generateFinishedHash(uint64_t clientRandom, uint64_t serverRandom)
{
    // Combine handshake messages into a single buffer
    std::vector<uint8_t> handshakeData;

    // Add client and server randoms
    auto clientRandomBytes = PRF::uint64ToByteVector(clientRandom);
    auto serverRandomBytes = PRF::uint64ToByteVector(serverRandom);

    handshakeData.insert(handshakeData.end(), clientRandomBytes.begin(), clientRandomBytes.end());
    handshakeData.insert(handshakeData.end(), serverRandomBytes.begin(), serverRandomBytes.end());

    // Generate hash
    return PRF::sha256(handshakeData);
}

bool DTLS::verifyFinishedMessage(const std::vector<char>& finishedMsg, uint64_t clientRandom, uint64_t serverRandom)
{
    auto expectedHash = generateFinishedHash(clientRandom, serverRandom);

    // Convert received message from char to uint8_t
    std::vector<uint8_t> receivedHash(finishedMsg.begin(), finishedMsg.end());

    return expectedHash == receivedHash;
}

std::pair<uint64_t, uint64_t> DTLS::clientHandshake(Communicator& comm)
{
    uint64_t random = static_cast<uint64_t>(generateRandomUint32());

    std::string name = PROTOCOL_VERSION;
    // send client hello
    ClientHello clientHelloMsg;
    clientHelloMsg.ProtocolVersion = name;
    clientHelloMsg.random = random;
    clientHelloMsg.session_id = generateRandomUint32();
    clientHelloMsg.cipher_suites_length = 2;    //2 algrothim for example
    clientHelloMsg.cipher_suites.push_back(0xC013); //  examples: 0xC013 = TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 
    clientHelloMsg.cipher_suites.push_back(0x0035); // 0x0035 = TLS_RSA_WITH_AES_128_CBC_SHA256

    comm.sendMessage(clientHelloMsg); // send hello messege
    std::cout << "send hello msg seccussfuly!" << std::endl;

    std::vector<char> helloFromSever = comm.receiveMessage();
    ServerHello serverHello = Deserializer::deserializeServerHello(helloFromSever);
    std::cout << "recieve hello msg seccussfuly!" << std::endl;

    return std::make_pair(clientHelloMsg.random, serverHello.random);
  
}


bool DTLS::sendCertificateMessegeToServer(Communicator& comm)
{
    randomRsa.generate_keys();
    randomRsa.print_keys();

    // Certificate for server(Child)
    Certificate cert;

    /*// get computer name
    char serverName[255];
    DWORD size = sizeof(serverName);
    GetComputerNameA(serverName, &size);
    cert.name = std::string(serverName);*/
    cert.name = "parent";

    //getCurrentDate
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now_tm);
    cert.departure_date = std::string(buffer); 


    // getExpirationDate
    auto now2 = std::chrono::system_clock::now();
    auto expiration_time = now2 + std::chrono::seconds(365 * 24 * 60 * 60);
    auto expiration_c = std::chrono::system_clock::to_time_t(expiration_time);
    std::tm* expiration_tm = std::localtime(&expiration_c);
    char buffer2[11];
    strftime(buffer2, sizeof(buffer2), "%Y-%m-%d", expiration_tm);
    cert.expiration_date = std::string(buffer2);// Get the expiration date (365 days ahead)


    cert.publickey = randomRsa.getPublicKey();
    std::string certificate_data = cert.name + cert.departure_date;

    std::cout << "certificate_data CLIENT SIDE : " << certificate_data <<std::endl;

    cert.digital_signature = randomRsa.signMessage(certificate_data);

    return comm.sendMessage(cert);
}

uint64_t DTLS::sendPremasterToClient(Communicator& comm)
{
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dis(0, UINT32_MAX);
    uint64_t rndNum = dis(gen);

    uint64_t encryptMsg = randomRsa.encrypt(rndNum);
    comm.sendMessage(encryptMsg);
    return rndNum;
}

std::pair<KeyUse::Keys, KeyUse::Keys> DTLS::generateKeyDerivation(uint64_t premasterSecret, uint64_t clientRandom, uint64_t serverRandom)
{
    std::vector<uint8_t> masterSecret = PRF::generateMasterSecret(
        premasterSecret,        // premaster secret
        clientRandom,              // client random
        serverRandom   // server random 
    );


    auto keys = PRF::DTLSKeyExpansion(masterSecret, clientRandom, serverRandom, 32, 16, 16);

    /*const char* keyNames[] = {
      "Client MAC Key",
      "Server MAC Key",
      "Client Encryption Key",
      "Server Encryption Key",
      "Client IV",
      "Server IV"
    };

    for (size_t i = 0; i < keys.size(); i++) {
        std::cout << keyNames[i] << ": ";
        for (uint8_t byte : keys[i]) {
            printf("%02x ", byte);
        }
        std::cout << std::endl;
    }*/

    KeyUse::Keys clientKeys = KeyUse::extractClientKeys(keys);
    KeyUse::Keys serverKeys = KeyUse::extractServerKeys(keys);

    return std::make_pair(clientKeys, serverKeys);
}

std::vector<char> DTLS::encryptMessege(int role, std::string msg) const
{
    KeyUse::Keys keys;
    std::vector<uint8_t> macKey;
    std::string messege = "";

    if (role == CLIENT_PARENT)
    {
        keys = this->keys.first;
        macKey = this->keys.second.mac_key;
    }
    else
    {
        keys = this->keys.second;
        macKey = this->keys.first.mac_key;
    }

    std::vector<uint8_t> messageData(msg.begin(), msg.end());
    std::vector<uint8_t> encryptedMsg2 = KeyUse::encrypt(
        messageData,
        keys.encryption_key,
        keys.iv
    );

    std::cout << "messege size without mac : " << messageData.size() << std::endl;

    std::vector<uint8_t> mac = KeyUse::sign(encryptedMsg2, macKey);

    encryptedMsg2.insert(encryptedMsg2.end(), mac.begin(), mac.end());
    std::vector<char> sendData(encryptedMsg2.begin(), encryptedMsg2.end());
    return sendData;
}


std::string DTLS::decryptMeessege(int role, std::vector<char> encryptedMsg, int bytesRead) const
{
    KeyUse::Keys keys;
    std::string messege = "";

    if (role == CLIENT_PARENT)
    {
        keys = this->keys.first;
    }
    else
    {
        keys = this->keys.second;
    }

    std::vector<uint8_t> encryptedData(encryptedMsg.begin(), encryptedMsg.begin() + bytesRead);

    std::cout << "\encryptedData : ";
    PRF::printByteVector(encryptedData, "");


    std::vector<uint8_t> receivedMac(encryptedData.end() - MAC_SIZE, encryptedData.end());

    std::cout << "\nReceived MAC: ";
    PRF::printByteVector(receivedMac, "");


    std::vector<uint8_t> encryptedContent(encryptedData.begin(), encryptedData.end() - MAC_SIZE);


    std::cout << "encryptedContent size : " << encryptedContent.size() << std::endl;
    std::cout << "\encryptedContent: ";
    PRF::printByteVector(encryptedContent, "");


    std::vector<uint8_t> calculatedMac = KeyUse::sign(encryptedContent, keys.mac_key);
    std::cout << "\calculatedMac MAC: ";
    PRF::printByteVector(calculatedMac, "");

    if (KeyUse::verify(encryptedContent, receivedMac, keys.mac_key)) {
        std::cout << "\nMAC verification successful!" << std::endl;

        std::vector<uint8_t> decryptedMsg = KeyUse::decrypt(
            encryptedContent,
            keys.encryption_key,
            keys.iv
        );

        for (uint8_t byte : decryptedMsg) {
            messege.push_back(static_cast<char>(byte));
        }
    }
    else {
        std::cout << "\nMAC verification failed!" << std::endl;
    }
    return messege;

}

void DTLS::printKeyMaterial()
{
   

}


uint32_t DTLS::generateRandomUint32()
{
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    return dist(generator);
}


//client
bool DTLS::runParentDTLS(Communicator& comm)
{
    std::pair<uint64_t, uint64_t> peersRandoms = clientHandshake(comm);
    sendCertificateMessegeToServer(comm);
    std::cout << "CertificateMessege send successfuly!" << std::endl;

    uint64_t encryptedPremasterSecret = comm.receive_msg_uint64_t(); // recieve the premaster
    std::cout << "prematser recieved successfuly!" << std::endl;

    uint64_t decryptedPremasterSecret = randomRsa.decrypt(encryptedPremasterSecret);
  
    keys =  generateKeyDerivation(decryptedPremasterSecret, peersRandoms.first, peersRandoms.second);



    // Generate and send Finished message
    auto finishedHash = generateFinishedHash(peersRandoms.first, peersRandoms.second);
    std::vector<char> encryptedFinished = encryptMessege(CLIENT_PARENT, std::string(finishedHash.begin(), finishedHash.end()));
    comm.sendBytes(encryptedFinished);
    std::cout << " finished messeges sent seccussfuly!" << std::endl;


    // Receive and verify server's Finished message
    auto serverFinished = comm.receiveMessage();
    std::cout << "Encrypted message size after sent: " << serverFinished.size() << std::endl;

    std::string decryptedServerFinished = decryptMeessege(CLIENT_PARENT, serverFinished, serverFinished.size());

    return verifyFinishedMessage(std::vector<char>(decryptedServerFinished.begin(), decryptedServerFinished.end()), peersRandoms.first, peersRandoms.second);
    
    std::cout << " finished checked sent seccussfuly!" << std::endl;

    return true;
}
