#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Communicator.h"
#include "SHA256.h"



#ifdef max
#undef max
#endif


void Communicator::sendAuthentication(const SOCKET& sock, AuthenticationRequest& authRequest) {
    std::vector<char> buffer(1024);
    
    // create json object for authentication
    nlohmann::json authMessage;
	authMessage["id"] = authRequest.id;
    authMessage["username"] = authRequest.username;
    authMessage["password"] = authRequest.password;
    authMessage["role"] = authRequest.role;
    authMessage["ip"] = authRequest.ip;

    std::string message = authMessage.dump();
    std::copy(message.begin(), message.end(), buffer.begin());
    send(sock, buffer.data(), buffer.size(), 0);
}



std::string Communicator::generateRandomSecret(int length)
{
    const char charset[] = "0123456789"; // Numeric secret
    std::string secret;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2); // Exclude null terminator

    for (int i = 0; i < length; ++i) {
        secret += charset[distribution(generator)];
    }
    return secret;
}

std::string Communicator::getLocalIPAddress()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        std::cerr << "gethostname failed: " << WSAGetLastError() << std::endl;
        return "";
    }

    struct hostent* hostEntry = gethostbyname(hostname);
    if (hostEntry == nullptr) {
        std::cerr << "gethostbyname failed: " << WSAGetLastError() << std::endl;
        return "";
    }

    // convert the first address from network byte order to presentation format
    struct in_addr* addr = (struct in_addr*)hostEntry->h_addr_list[0];
    return inet_ntoa(*addr);
}

void Communicator::runAsPeer(const std::string& peer2IP, const SOCKET& connectionToSignalingServerSocket, const std::string& role)
{
    std::vector <char> buffer(1024);

    AuthenticationRequest authRequest;

    std::cout << "Would you like to (login) or (register)? For login enter 1, for register enter 0: ";
    std::cin >> authRequest.id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter your username: ";
    std::getline(std::cin, authRequest.username);

    std::cout << "Enter your password: ";
    std::getline(std::cin, authRequest.password);

    authRequest.role = role;

    //std::string ip = getLocalIPAddress(); // for when i need communiaction otside my computer(for now it's 127.0.0.1)
    std::string ip = "127.0.0.1";
    authRequest.ip = ip; //


    sendAuthentication(connectionToSignalingServerSocket, authRequest);


    // recieve response from server that client approved
    int bytesReceived = recv(connectionToSignalingServerSocket, buffer.data(), buffer.size(), 0);
    AuthResponse authResponse = Deserializer::deserializeAuthResponse(buffer);


    // Print the struct values
    std::cout << "AuthResponse struct:\n";
    std::cout << "Status: " << authResponse.status << "\n";
    std::cout << "Message: " << authResponse.message << "\n";

    if (role == "Parent")
	{
		runAsParent(peer2IP, connectionToSignalingServerSocket);
	}
	else
	{
		runAsChild(connectionToSignalingServerSocket);
	}

}

void Communicator::runAsParent(const std::string& peer2IP, const SOCKET& connectionToSignalingServerSocket) {
   
    std::vector <char> buffer(1024);

    // needs to add send secret
       // Generate a random secret of 8 digits
    std::string secret = generateRandomSecret(8);

    std::cout <<  "Secret : " << secret << std::endl;

    // Hash the secret
    std::string hashedSecret = SHA256::sha256(secret);

    std::cout << "Hashed Secret (SHA-256): " << hashedSecret << std::endl;
    char numberOfKids = '0';
    std::cout << "Enter number of kids: ";
    std::cin >> numberOfKids;

    std::cout << hashedSecret.length() << std::endl;


    hashedSecret = numberOfKids + hashedSecret;

    std::vector<char> hashedSecretBuffer(hashedSecret.length());

    std::copy(hashedSecret.begin(), hashedSecret.end(), hashedSecretBuffer.begin());
    send(connectionToSignalingServerSocket, hashedSecretBuffer.data(), hashedSecretBuffer.size(), 0);
   

    // recieve response if bind aproved 
    int bytesReceived = recv(connectionToSignalingServerSocket, buffer.data(), buffer.size(), 0);

    AuthResponse authResponse = Deserializer::deserializeAuthResponse(buffer);


    // Print the struct values
    std::cout << "AuthResponse struct:\n";
    std::cout << "Status: " << authResponse.status << "\n";
    std::cout << "Message: " << authResponse.message << "\n";


    //send SDP offer to server
    
    SDP sdpOffer;
    sdpOffer.sessionID = rand();  //random session ID
    sdpOffer.sessionVersion = 0;  // Start versioning from 0

    //std::string ip = getLocalIPAddress(); // for when i need communiaction otside my computer(for now it's 127.0.0.1)
    std::string ip = "127.0.0.1";

    sdpOffer.candidateIP = ip;
    sdpOffer.candidatePort = 8080;
    /*
    Audio Codecs
    111: Opus - A versatile audio codec ideal for both music and voice, offering low latency and high quality.
    103: ISAC - A codec optimized for wideband audio, perfect for clear voice communication.
    Video Codecs
    100: VP8 - An open-source codec providing good quality video compression for real-time applications.
    101: H.264 - A widely used codec known for high-quality video at lower bitrates, though not royalty-free.
    102: VP9 - An efficient successor to VP8, offering improved compression and quality, especially for high resolutions.
    103: AV1 - A modern, royalty-free codec designed for high-efficiency video streaming, outperforming older codecs.
    */
    sdpOffer.mediaCapabilities.audioCodecs = { 111, 103 };  // Opus and ISAC
    sdpOffer.mediaCapabilities.videoCodecs = { 100, 101, 102, 103 };  // VP8, H.264, VP9, AV1
    sdpOffer.transportInfo = "RTP";

    send(connectionToSignalingServerSocket, Serializer::serializeSDP(sdpOffer).data(), Serializer::serializeSDP(sdpOffer).size(), 0);

    //get SDP answer
    bytesReceived = recv(connectionToSignalingServerSocket, buffer.data(), buffer.size(), 0);
    if (bytesReceived > 0)
    {
        SDP sdpAnswer = Deserializer::deserializeSDP(buffer);

        // PRINTING FOR DEBUG

		std::cout << "sdpAnswer from childdd::::::::::::::::::::::::::" << std::endl;
        std::cout << sdpAnswer.candidateIP << std::endl;
        std::cout << sdpAnswer.candidatePort << std::endl;
        std::cout << sdpAnswer.sessionID << std::endl;

    }
    std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get();
    
}


void Communicator::runAsChild(const SOCKET& connectionToSignalingServerSocket) {
  
    std::cout << "Enter secret: ";

    std::string secret;
    std::getline(std::cin, secret);

	// Hash the secret
	std::string hashedSecret = SHA256::sha256(secret);

	std::cout << "Hashed Secret (SHA-256): " << hashedSecret << std::endl;
    hashedSecret = '1' + hashedSecret;

    std::vector<char> hashedSecretBuffer(hashedSecret.length());
    std::cout << hashedSecret.length() << std::endl;
    std::copy(hashedSecret.begin(), hashedSecret.end(), hashedSecretBuffer.begin());
    send(connectionToSignalingServerSocket, hashedSecretBuffer.data(), hashedSecretBuffer.size(), 0);


    // recieved bind response
    std::vector <char> buffer(1024);
    int bytesReceived = recv(connectionToSignalingServerSocket, buffer.data(), buffer.size(), 0);
    AuthResponse authResponse = Deserializer::deserializeAuthResponse(buffer);


    // Print the struct values
    std::cout << "AuthResponse struct:\n";
    std::cout << "Status: " << authResponse.status << "\n";
    std::cout << "Message: " << authResponse.message << "\n";


    
    // waiting to get sdp offer
    bytesReceived = recv(connectionToSignalingServerSocket, buffer.data(), buffer.size(), 0);


    if (bytesReceived > 0)
    {
        SDP sdpOffer = Deserializer::deserializeSDP(buffer);

        std::cout << "sdpOffer from parenttttttttt ::::::::::::::::::::::::::" << std::endl;

        // PRINTING FOR DEBUG
        std::cout << sdpOffer.candidateIP << std::endl;
        std::cout << sdpOffer.candidatePort << std::endl;
        std::cout << sdpOffer.sessionID << std::endl;


        SDP answer;
        answer.sessionID = sdpOffer.sessionID;  // Keep the same session ID
        answer.sessionVersion = sdpOffer.sessionVersion + 1;  // Increment version

        //std::string ip = getLocalIPAddress(); // for when i need communiaction otside my computer(for now it's 127.0.0.1)
        std::string ip = "127.0.0.1";

        answer.candidateIP = ip;
        answer.candidatePort = 8080;
        answer.mediaCapabilities.audioCodecs = sdpOffer.mediaCapabilities.audioCodecs;  // copy from the offer to be similar
        answer.mediaCapabilities.videoCodecs = sdpOffer.mediaCapabilities.videoCodecs;  // copy from the offer to be similar
        answer.transportInfo = "RTP";
        send(connectionToSignalingServerSocket, Serializer::serializeSDP(sdpOffer).data(), Serializer::serializeSDP(sdpOffer).size(), 0);


    }
    std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get();
}
