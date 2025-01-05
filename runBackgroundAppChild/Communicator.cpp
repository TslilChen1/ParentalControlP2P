#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Communicator.h"
#include <iostream>
#include "Requests.h"
#include "Serializer.h"
#include "RemoteControlingRequestHandler.h"
#include "DTLS.h"

#pragma comment(lib, "Ws2_32.lib")

#ifdef max
#undef max
#endif

#define PARENT 1
#define CHILD 0

bool Communicator::connectToOtherPeer(int port)
{

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
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  


    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (listen(sock, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    clientSock = accept(sock, (struct sockaddr*)&clientAddr, &clientSize);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "Accept failed!" << std::endl;
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    this->_sock = clientSock; 
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

RequestInfo Communicator::parseRequest(const std::vector<char>& buffer, int bytesReceived, const DTLS& dtls)
{
    std::string decryptedMessage = dtls.decryptMeessege(CHILD, buffer, bytesReceived);
    std::vector<char> decryptedBuffer(decryptedMessage.begin(), decryptedMessage.end());

    std::cout << "Debug: Parsing request with " << bytesReceived << " bytes." << std::endl;

    RequestInfo request;

    request.id = static_cast<uint8_t>(decryptedBuffer[0]);
    request.receivalTime = std::time(nullptr);
    //request.receivalTime = 0;

    bytesReceived -= 32;
    std::cout << "bytesReceived : " << bytesReceived - 32 << std::endl;

    request.buffer = std::vector<char>(decryptedBuffer.begin() + 1, decryptedBuffer.begin() + bytesReceived);

    //request.buffer = std::vector<char>(decryptedBuffer.begin() + 1, decryptedBuffer.begin() + 4);

    std::cout << "Debug: Parsed request with ID: " << static_cast<int>(request.id)
        << " and data size: " << request.buffer.size() << std::endl;

    return request;
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



bool Communicator::sendMessage(ServerHello& msg)
{
    try {
        std::vector<char> serializedData = Serializer::serializeServerHello(msg);

        std::cout << "Debug - ServerHello before send:" << std::endl;
        std::cout << "Buffer size: " << serializedData.size() << std::endl;
        std::cout << "Protocol Version: " << msg.ProtocolVersion << std::endl;
        std::cout << "Random: " << msg.random << std::endl;
        std::cout << "Session ID: " << msg.session_id << std::endl;

        if (serializedData.empty()) {
            std::cerr << "Serialization failed - empty data" << std::endl;
            return false;
        }

        if (serializedData.data() == nullptr) {
            std::cerr << "Critical error: Null buffer" << std::endl;
            return false;
        }

        if (_sock == INVALID_SOCKET) {
            std::cerr << "Invalid socket" << std::endl;
            return false;
        }

        size_t totalSent = 0;
        const size_t CHUNK_SIZE = 1024;
        int retryCount = 0;
        const int MAX_RETRIES = 3;

        while (totalSent < serializedData.size() && retryCount < MAX_RETRIES) {
            size_t remaining = serializedData.size() - totalSent;
            size_t currentChunk = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

            int result = send(_sock,
                serializedData.data() + totalSent,
                static_cast<int>(currentChunk),
                0);

            if (result == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    //Sleep(100);  // Short sleep before retry
                    retryCount++;
                    continue;
                }
                std::cerr << "Send failed with error: " << error << std::endl;
                return false;
            }

            if (result == 0) {
                std::cerr << "Connection closed" << std::endl;
                return false;
            }

            totalSent += result;
            retryCount = 0;  // Reset retry count on successful send
        }

        if (totalSent == serializedData.size()) {
            std::cout << "Successfully sent " << totalSent << " bytes" << std::endl;
            return true;
        }
        else {
            std::cerr << "Failed to send complete message after retries" << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during send: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception during send" << std::endl;
        return false;
    }
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

bool Communicator::runChildMain(DTLS& dtls)
{
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
    std::cin.ignore();
    std::cin.ignore();
    std::cin.get(); std::cin.get();
    return true;
}
