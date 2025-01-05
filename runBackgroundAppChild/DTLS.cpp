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

std::pair<uint64_t, uint64_t>  DTLS::serverHandshake(Communicator& comm)
{
	std::vector<char> helloFromClient = comm.receiveMessage(); // recieve hello messege from the Parent
    std::cout << "recieve hello msg seccussfuly!" << std::endl;

	ClientHello clientHelloFromClient = Deserializer::deserializeClientHello(helloFromClient);

    ServerHello serverHello;
    uint64_t random = static_cast<uint64_t>(generateRandomUint32()); ;

    serverHello.ProtocolVersion = PROTOCOL_VERSION;
    serverHello.random = random;
    serverHello.session_id = clientHelloFromClient.session_id;

    if (!clientHelloFromClient.cipher_suites.empty()) {
        serverHello.cipher_suite = clientHelloFromClient.cipher_suites.back();
    }
    else
    {
        serverHello.cipher_suite = 0;
    }


    comm.sendMessage(serverHello); // send server hello to client
    std::cout << "send hello msg seccussfuly!" << std::endl;

    return std::make_pair(clientHelloFromClient.random, serverHello.random);


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

// server
bool DTLS::runChildDTLS(Communicator& comm)
{
    std::pair<uint64_t, uint64_t> peersRandoms = serverHandshake(comm);
    
    std::vector<char> certificateOfServer = comm.receiveMessage();
    std::cout << "CertificateMessege recieved successfuly!" << std::endl;
    Certificate certificateFromClient = Deserializer::deserializeCertificate(certificateOfServer);

    randomRsa.setPublicKey(certificateFromClient.publickey);

    if (!(this->randomRsa.verify(certificateFromClient.digital_signature)))
    {
        return false;
    }
    std::cout << "verify of client signature passed successfuly!" << std::endl;

    uint64_t premster = sendPremasterToClient(comm);
    std::cout << "prematser send successfuly!" << std::endl;

    keys = generateKeyDerivation(premster, peersRandoms.first, peersRandoms.second);
    


    // Receive and verify client's Finished message
    auto clientFinished = comm.receiveMessage();
    std::string decryptedClientFinished = decryptMeessege(!CLIENT_PARENT, clientFinished, clientFinished.size());

    if (!verifyFinishedMessage(std::vector<char>(decryptedClientFinished.begin(), decryptedClientFinished.end()),peersRandoms.first, peersRandoms.second))
    {
        return false;
    }

    std::cout << " finished messeges checked seccussfuly!" << std::endl;

    // Send server Finished message
    auto finishedHash = generateFinishedHash(peersRandoms.first, peersRandoms.second);
    std::vector<char> encryptedFinished = encryptMessege(SERVER_CHILD, std::string(finishedHash.begin(), finishedHash.end()));
    std::cout << "Encrypted message size before sent: " << encryptedFinished.size() << std::endl;


    return comm.sendBytes(encryptedFinished);
    std::cout << " finished messeges sent seccussfuly!" << std::endl;




    return true;

}