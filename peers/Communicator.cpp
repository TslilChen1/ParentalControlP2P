#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Communicator.h"
#include "DTLS.h"

#ifndef MAC_SIZE
#define MAC_SIZE 32
#endif
#include "Deserializer.h"

#ifdef max
#undef max
#endif

#define PARENT 1
#define CHILD 0
#define PORT2 12355

std::wstring BuildCommandLine(const std::string& exePath, int port) {
    std::wstring commandLine;
    commandLine.reserve(exePath.length() + 20);
    commandLine = std::wstring(exePath.begin(), exePath.end());
    if (commandLine.back() != L' ') {
        commandLine += L" ";
    }
    commandLine += std::to_wstring(port);
    return commandLine;
}

bool RunWindowService(const std::string& exePath, int port) {
    std::wstring cmdLine = BuildCommandLine(exePath, port);
    std::vector<wchar_t> cmdLineCopy(cmdLine.begin(), cmdLine.end());
    cmdLineCopy.push_back(0); // Null terminator

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcessW(NULL, cmdLineCopy.data(),
        NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool isValidUrl(const std::string& url)
{
    std::vector<std::string> validExtensions = { ".com", ".net", ".org", ".co.il", ".edu", ".gov", ".io" , ".org.il" };

    for (const auto& ext : validExtensions) {
        if (url.rfind(ext) == url.length() - ext.length()) {
            return true;
        }
    }

    return false;
}



void Communicator::printMenu()
{
    std::cout << "Enter a command- " << std::endl;
    std::cout << "0 for  exit" << std::endl;
    std::cout << "1 for TURN_OFF" << std::endl;
    std::cout << "2 for KEY_BOARD_READING" << std::endl;
    std::cout << "4 for BLOCK_WEB" << std::endl;

}

void Communicator::sendAuthentication(const SOCKET& sock, AuthenticationRequest& authRequest) {
    std::vector<char> buffer(1024);
    
    // create json object for authentication
    nlohmann::json authMessage;
	authMessage["id"] = authRequest.id;
    authMessage["username"] = authRequest.username;
    authMessage["password"] = authRequest.password;
    authMessage["role"] = authRequest.role;

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

bool Communicator::connectToOtherPeer(const std::string& role)
{
    Sleep(60); // wait for the window service to run
    WSADATA wsaData;
    SOCKET sock, clientSock;
    sockaddr_in serverAddr, clientAddr;
    int clientSize = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return INVALID_SOCKET;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(99999);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return false;
    }
    this->_sock = sock;
    
}



bool Communicator::sendRequest(const RequestInfo& request, const DTLS& dtls)
{
    int result;
    std::vector<char> data;
    data.push_back(static_cast<char>(request.id));
    data.insert(data.end(), request.buffer.begin(), request.buffer.end());

    std::vector<char> encryptedData = dtls.encryptMessege(PARENT, std::string(data.begin(), data.end()));

    result = send(_sock, encryptedData.data(), encryptedData.size(), 0);

    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    else {
        std::cout << "Debug: Request sent successfully." << std::endl;
        return true;
    }
    return false;

}

Communicator::~Communicator()
{
    closesocket(_sock); 
    WSACleanup();
}

void Communicator::runAsPeer(const SOCKET& connectionToSignalingServerSocket, const std::string& role)
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
		runAsParent(connectionToSignalingServerSocket);
	}
	else
	{
		runAsChild(connectionToSignalingServerSocket);
	}

}

void Communicator::runAsParent(const SOCKET& connectionToSignalingServerSocket)
{
    std::vector <char> buffer(1024);

    // needs to add send secret
       // Generate a random secret of 8 digits
    std::string secret = generateRandomSecret(8);

    std::cout << "Secret : " << secret << std::endl;

    // Hash the secret
    std::string hashedSecret = SHA256::sha256(secret);

    std::cout << "Hashed Secret (SHA-256): " << hashedSecret << std::endl;
    char numberOfKids = '0';
    std::cout << "Enter number of kids: ";
    std::cin >> numberOfKids;

    std::cout << hashedSecret.length() << std::endl;


    hashedSecret = numberOfKids + hashedSecret;

    std::cout << hashedSecret << std::endl;
    std::cout << hashedSecret.length() << std::endl;

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
    //std::string ip = "127.0.0.1";

    sdpOffer.candidateIP = "0.0.0.0";
    sdpOffer.candidatePort = PORT2;
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
    
    std::string ip = StunRequests::runAngGetIP();
    std::cout << "ip: " << ip << std::endl;
   
    /*IceCandidate i =
    {
        // srflx means that machine behind NAT
        ip, 8888, "tcp", "srflx"
    uint32_t priority;
    std::string foundation;
    int component_id;
    }*/

    IceCandidate i = {
    ip,
    PORT2,           // port
    "tcp",          // protocol
    "srflx",        // srflx means that machine behind NAT
    123456,         // priority
    "candidate1",   // foundation
    1               //  for exapmle RTP  1, RTCP 2
    };
    //////////////////////////////////////////////////////////////////////////////////////


    connectToOtherPeer("Parent");;
    
    DTLS dtls;


    dtls.runParentDTLS(*this);
    //dtls.printKeyMaterial();


    std::cout << "\n\n\n\n";

    /*
    std::vector<char> msg = dtls.encryptMessege(PARENT, "hello from parent!");
    this->sendBytes(msg);
    */

    std::cout << "ftls finish parent" << std::endl;

    while (true) {
        int seconds = 0;
        RequestInfo request;
        int input;
        printMenu();
        std::cin >> input;
        std::string webName;

        switch (input)
        {
        case 0:
            std::cout << "Debug: Exiting client loop." << std::endl;
            break;

        // 1
        case TURN_OFF_COMPUTER_REQUEST:
            request.id = TURN_OFF_COMPUTER_REQUEST;
            request.buffer = { 'O', 'F', 'F' };
            break;

        /*case KEY_BOARD_READING_REQUEST:
        {
            seconds = 0;
            std::cout << "how many seconds to read?" << std::endl;
            std::cin >> seconds;
            request.id = KEY_BOARD_READING_REQUEST;
            KeyBoardReadingRequest keyBoardReadingRequest;
            keyBoardReadingRequest.timeInSecondToRead = seconds;

            request.buffer = Serializer::Serialize(keyBoardReadingRequest);

            break;
        }*/

        // 4
        case BLOCK_WEB_REQUEST:
        {
            std::cout << "enter the name of web you want to block " << std::endl;
            std::cin >> webName;

            while (!isValidUrl(webName))
            {
                std::cout << "Invalid URL, please enter a valid URL with an appropriate extension." << std::endl;
                std::cout << "enter the name of web you want to block " << std::endl;
                std::cin >> webName;
            }

            std::cout << "web name that just been typed : " << webName << std::endl;
            request.id = BLOCK_WEB_REQUEST;

            //BlockingWebRequest blockWebRequest;
            //blockWebRequest.webName = webName;

            request.buffer = Serializer::Serialize(webName);
            break;
        }

        default:
            std::cerr << "Unknown command: " << input << std::endl;
            continue;
        }

        /*
        if (input == KEY_BOARD_READING_REQUEST)
        {
            Sleep(seconds * 1000);
        }*/

        sendRequest(request,dtls); 

        std::vector<char> responseBuffer(1024);
        int bytesReceived = recv(_sock, responseBuffer.data(), responseBuffer.size(), 0);
        if (bytesReceived == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            std::cerr << "Receive failed with error code: " << errorCode << std::endl;
            break;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by peer." << std::endl;
            break;
        }


        std::string decryptMessege = dtls.decryptMeessege(PARENT, responseBuffer, bytesReceived);
        std::cout << "Received response: " << decryptMessege << std::endl;
    }


    std::cin.ignore();
    std::cin.ignore();
    std::cin.get(); std::cin.get();


}

void Communicator::runAsChild(const SOCKET& connectionToSignalingServerSocket) {
  
    std::cout << "Enter secret: ";

    std::string hashedSecret;
    std::getline(std::cin, hashedSecret);

	// Hash the secret
	//std::string hashedSecret = SHA256::sha256(secret);


	//std::cout << "Hashed Secret (SHA-256): " << hashedSecret << std::endl;
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

    SDP sdpOffer;
    if (bytesReceived > 0)
    {
        sdpOffer = Deserializer::deserializeSDP(buffer);

        std::cout << "sdpOffer from parenttttttttt ::::::::::::::::::::::::::" << std::endl;

        // PRINTING FOR DEBUG
        std::cout << sdpOffer.candidateIP << std::endl;
        std::cout << sdpOffer.candidatePort << std::endl;
        std::cout << sdpOffer.sessionID << std::endl;


        SDP answer;
        answer.sessionID = sdpOffer.sessionID;  // Keep the same session ID
        answer.sessionVersion = sdpOffer.sessionVersion + 1;  // Increment version

        //std::string ip = getLocalIPAddress(); // for when i need communiaction otside my computer(for now it's 127.0.0.1)
        //std::string ip = "127.0.0.1";

        answer.candidateIP = "0.0.0.0";
        answer.candidatePort = sdpOffer.candidatePort;
        answer.mediaCapabilities.audioCodecs = sdpOffer.mediaCapabilities.audioCodecs;  // copy from the offer to be similar
        answer.mediaCapabilities.videoCodecs = sdpOffer.mediaCapabilities.videoCodecs;  // copy from the offer to be similar
        answer.transportInfo = "RTP";
        send(connectionToSignalingServerSocket, Serializer::serializeSDP(sdpOffer).data(), Serializer::serializeSDP(sdpOffer).size(), 0);


    }


	std::string ip = StunRequests::runAngGetIP();
    std::cout << ip;
    //std::cin.get(); std::cin.get(); std::cin.get(); std::cin.get();

    IceCandidate i = {
   ip,
   PORT2,           // port
   "tcp",          // protocol
   "srflx",        // srflx means that machine behind NAT
   123456,         // priority, in this project everybody will be connected so the number does not important
   "candidate2",   // foundation
   1               //  for exapmle RTP  1, RTCP 2
    };

    //////////////////////////////////////////////////////////////////////////////////////
    // 
    // needs to add code that runs the window service and the window service will run the 

     
    //std::string exePath = "runWindowService.exe ";
                     
    RunWindowService("C:\\Users\\Magshimim\\Documents\\Magshimim\\or-akiva-504-parentalcontrolp2p\\peers\\runWindowService.exe", sdpOffer.candidatePort);

    /*std::wstring wExePath(exePath.begin(), exePath.end());
    int candidatePort = sdpOffer.candidatePort;

    std::cout << "the service start listening in port" << sdpOffer.candidatePort << "............" << std::endl;
    wExePath += std::to_wstring(candidatePort);

    LPWSTR commandLine = &wExePath[0];  

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "The program is running!" << std::endl;

        WaitForSingleObject(pi.hProcess, INFINITE);
        std::cout << "The program has finished." << std::endl;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::cerr << "Failed to create process." << std::endl;
    }*/



    /*
    connectToOtherPeer("Child");;
    
    DTLS dtls;

    dtls.runChildDTLS(*this);
    //dtls.printKeyMaterial();


    std::cout << "\n\n\n\n";

    
    std::vector<char> msg = this->receiveMessage();
    std::cout << "decryptMeessege : " << dtls.decryptMeessege(CHILD, msg) << std::endl;
    

    
    std::vector<char> recieveBuffer(1024);
    std::shared_ptr<IRequestHandler> handler = std::make_shared<RemoteControlingRequestHandler>();

    while (true) {
        std::cout << "Debug: Waiting to receive data from peer..." << std::endl;
        int bytesReceived = recv(_sock, recieveBuffer.data(), recieveBuffer.size(), 0);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed or client disconnected. Error code: " << WSAGetLastError() << std::endl;
            break;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by peer." << std::endl;
            break;
        }
        
        std::cout << "Debug: Received " << bytesReceived << " bytes from peer." << std::endl;

        RequestInfo request = parseRequest(recieveBuffer, bytesReceived, dtls);

        IRequestHandler::RequestResult result = handler->handleRequest(request);

        if (!result.response.empty()) {
            std::cout << "Debug: Sending response to peer." << std::endl;
            std::vector<char> response = result.response;
            std::string message(response.begin(), response.end());
            std::vector<char> encryptedVector = dtls.encryptMessege(CHILD, message);
            int bytesSent = send(_sock, encryptedVector.data(), encryptedVector.size(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "Send failed with error code: " << WSAGetLastError() << std::endl;
                break;
            }
            else {
                std::cout << "Debug: Sent " << bytesSent << " bytes to peer." << std::endl;
            }
        }
    }

    std::cout << "Debug: Closing client socket." << std::endl;




    std::cin.ignore();
    std::cin.ignore();
    std::cin.ignore();
    std::cin.ignore();
    std::cin.get(); std::cin.get();*/
   
}



bool Communicator::sendMessage(uint64_t msg)
{
    if (_sock == INVALID_SOCKET) {
        std::cerr << "Socket is not connected!" << std::endl;
        return false;
    }

    int result = send(_sock, reinterpret_cast<const char*>(&msg), sizeof(msg), 0);

    if (result == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Message sent successfully: " << msg << std::endl;
    return true;
}

bool Communicator::sendMessage(std::pair<uint64_t, uint64_t> publicKey)
{
    if (_sock == INVALID_SOCKET) {
        std::cerr << "Socket is not connected!" << std::endl;
        return false;
    }

    // Prepare the buffer
    uint64_t publicKeyBuffer[2] = { publicKey.first, publicKey.second };

    // Send the buffer
    int bytesSent = send(_sock, reinterpret_cast<const char*>(publicKeyBuffer), sizeof(publicKeyBuffer), 0);

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Message sent successfully: (" << publicKey.first << ", " << publicKey.second << ")" << std::endl;
    return true;
}

uint64_t Communicator::receive_msg_uint64_t()
{
    if (_sock == INVALID_SOCKET) {
        std::cerr << "Socket is not connected!" << std::endl;
        return 0;
    }

    uint64_t msg = 0;
    int bytesReceived = recv(_sock, reinterpret_cast<char*>(&msg), sizeof(msg), 0);

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Failed to receive message: " << WSAGetLastError() << std::endl;
        return 0;
    }
    else if (bytesReceived == 0) {
        std::cerr << "Connection closed by the peer." << std::endl;
        return 0;
    }

    std::cout << "Message received successfully: " << msg << std::endl;
    return msg;
}

std::pair<uint64_t, uint64_t> Communicator::receive_msg_publicKey()
{
    if (_sock == INVALID_SOCKET) {
        std::cerr << "Socket is not connected!" << std::endl;
        return { 0, 0 };
    }

    // Prepare a buffer for the incoming pair
    uint64_t publicKeyBuffer[2];
    int bytesReceived = recv(_sock, reinterpret_cast<char*>(publicKeyBuffer), sizeof(publicKeyBuffer), 0);

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Failed to receive message: " << WSAGetLastError() << std::endl;
        return { 0, 0 };
    }
    else if (bytesReceived == 0) {
        std::cerr << "Connection closed by the peer." << std::endl;
        return { 0, 0 };
    }

    std::cout << "Message received successfully: (" << publicKeyBuffer[0] << ", " << publicKeyBuffer[1] << ")" << std::endl;
    return { publicKeyBuffer[0], publicKeyBuffer[1] };
}

bool Communicator::sendMessage(ClientHello& msg)
{
    std::vector<char> serializedData = Serializer::serializeClientHello(msg);
    return sendBytes(serializedData);
}


bool Communicator::sendBytes(std::vector<char>& buffer)
{
    if (_sock == INVALID_SOCKET) {
        std::cerr << "Socket is not connected!" << std::endl;
        return false;
    }

    size_t totalBytesSent = 0;
    size_t bytesRemaining = buffer.size();

    // Add buffer size validation
    if (buffer.empty()) {
        std::cerr << "Empty buffer provided!" << std::endl;
        return false;
    }

    while (totalBytesSent < buffer.size()) {
        // Send in chunks, handling partial sends
        int bytesSent = send(_sock,
            buffer.data() + totalBytesSent,
            static_cast<int>(bytesRemaining),
            0);

        if (bytesSent == SOCKET_ERROR) {
            int error = WSAGetLastError();
            // Handle specific error cases
            if (error == WSAEWOULDBLOCK || error == WSAEINTR) {
                // Recoverable errors - retry
                Sleep(1);
                continue;
            }

            std::cerr << "Send failed with error: " << error << std::endl;
            return false;
        }

        if (bytesSent == 0) {
            std::cerr << "Connection closed by peer" << std::endl;
            return false;
        }

        totalBytesSent += bytesSent;
        bytesRemaining -= bytesSent;
    }

    std::cout << "Successfully sent " << totalBytesSent << " bytes" << std::endl;
    return true;
}
bool Communicator::sendMessage(Certificate& msg)
{
    std::vector<char> serializedData = Serializer::serializeCertificate(msg);
    return sendBytes(serializedData);
}


std::vector<char> Communicator::receiveMessage()
{
    std::vector<char> buffer(1024);
    int bytesReceived = recv(_sock, buffer.data(), 1024, 0);

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error in recv(): " << WSAGetLastError() << std::endl;
        {
            return {};
        }
    }

    if (bytesReceived == 0) {
        std::cout << "Connection closed by the peer." << std::endl;
        return {};
    }

    buffer.resize(bytesReceived);


    return buffer;
}

