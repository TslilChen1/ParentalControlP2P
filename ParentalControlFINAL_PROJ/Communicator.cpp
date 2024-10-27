#include "Communicator.h"


#include <iostream>
#include <thread>
#include <exception>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include "RemoteControlingRequestHandler.h"
#include <string>
#include <stdexcept>
#include "Requsests.h"
#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define SHUT_WR 1

Communicator::Communicator()
    : m_handlerFactory(std::make_unique<RemoteControlingRequestHandler>()),
    m_connectionEstablished(false) {
    std::cout << "Debug: Communicator constructor called." << std::endl;
}

/*Communicator::~Communicator() {
    std::cout << "Debug: Communicator destructor called." << std::endl;
    if (m_socket != INVALID_SOCKET) {
        std::cout << "Debug: Closing socket." << std::endl;
        closesocket(m_socket);
    }
    std::cout << "Debug: Cleaning up Winsock." << std::endl;
    WSACleanup();
}*/

void Communicator::runAsServer() {
    std::cout << "Debug: Starting runAsServer." << std::endl;
    try {
        WSADATA wsa_data = {};
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
            throw std::exception("WSAStartup Failed");
        std::cout << "Debug: WSAStartup succeeded." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "WSAStartup Failed" << std::endl;
        exit(1);
    }

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
    std::cout << "Debug: Socket created successfully." << std::endl;

    bindAndListen();

    std::cout << "Debug: Waiting for peer to connect..." << std::endl;
    SOCKET clientSocket = accept(m_socket, NULL, NULL);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection. Error code: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    std::cout << "Debug: peer accepted. Handling peer." << std::endl;
    handleNewClient(clientSocket);
}

void Communicator::runAsClient(const std::string& serverAddress) {
    std::cout << "Debug: Starting runAsClient with server address: " << serverAddress << std::endl;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error code: " << result << std::endl;
        return;
    }
    std::cout << "Debug: WSAStartup succeeded." << std::endl;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
    std::cout << "Debug: Socket created successfully." << std::endl;

    sockaddr_in serverInfo = { 0 };
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(PORT);

    if (inet_pton(AF_INET, serverAddress.c_str(), &serverInfo.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << serverAddress << std::endl;
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    if (connect(m_socket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        std::cerr << "Connection failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(m_socket);
        WSACleanup();
        return;
    }

    std::cout << "Debug: Connected to peer at " << serverAddress << ":" << PORT << std::endl;

    while (true) {
        int seconds = 0;
        RequestInfo request;
        int input;
        printMenu();
        std::cin >>  input;
        std::string webName;

        switch (input)
        {
        case 0:
            std::cout << "Debug: Exiting client loop." << std::endl;
            break;

        case TURN_OFF_COMPUTER_REQUEST:
            request.id = TURN_OFF_COMPUTER_REQUEST;
            request.buffer = { 'O', 'F', 'F' };
            break;

        case KEY_BOARD_READING_REQUEST:
        {
            seconds = 0;
            std::cout << "how many seconds to read?" << std::endl;
            std::cin >> seconds;
            request.id = KEY_BOARD_READING_REQUEST;
            KeyBoardReadingRequest keyBoardReadingRequest;
            keyBoardReadingRequest.timeInSecondToRead = seconds;
            request.buffer = Serialize(keyBoardReadingRequest);
            break;
        }

        case BLOCK_WEB_REQUEST:
        {
            std::cout << "enter the name of web you want to block " << std::endl;
            std::cin >> webName;
            request.id = BLOCK_WEB_REQUEST;
            BlockingWebRequest blockWebRequest;
            blockWebRequest.webName = webName;
            request.buffer = Serialize(blockWebRequest);
            break;
        }

        default:
            std::cerr << "Unknown command: " << input << std::endl;
            continue;
        }


        if (input == KEY_BOARD_READING_REQUEST)
        {
            Sleep(seconds * 1000);
        }

        std::vector<char> responseBuffer(1024);
        int bytesReceived = recv(m_socket, responseBuffer.data(), responseBuffer.size(), 0);
        if (bytesReceived == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            std::cerr << "Receive failed with error code: " << errorCode << std::endl;
            break;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by peer." << std::endl;
            break;
        }

        std::string response(responseBuffer.begin(), responseBuffer.begin() + bytesReceived);
        std::cout << "Received response: " << response << std::endl;
    }

    std::cout << "Debug: Closing client socket." << std::endl;
    closesocket(m_socket);
    WSACleanup();
    std::cin.get();  

}

void Communicator::printMenu()
{
    std::cout << "Enter a command- " << std::endl;
    std::cout << "0 for  exit" << std::endl;
    std::cout << "1 for TURN_OFF" << std::endl;
	std::cout << "2 for KEY_BOARD_READING" << std::endl;
    std::cout << "4 for BLOCK_WEB" << std::endl;



}

void Communicator::bindAndListen() {
    std::cout << "Debug: Starting bindAndListen." << std::endl;
    sockaddr_in serverInfo = { 0 };
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(PORT);
    serverInfo.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_socket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Bind failed: " + std::to_string(WSAGetLastError()));
    }
    std::cout << "Debug: Bind successful." << std::endl;

    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Listen failed: " + std::to_string(WSAGetLastError()));
    }
    std::cout << "Debug: Listening on port " << PORT << std::endl;
}

std::vector<char> Communicator::Serialize(const BlockingWebRequest& request)
{
    std::vector<char> buffer(sizeof(request));
    std::memcpy(buffer.data(), &request, sizeof(request));
    return buffer;
}

std::vector<char> Communicator::Serialize(const KeyBoardReadingRequest& request)
{
    std::vector<char> buffer(sizeof(request));
    std::memcpy(buffer.data(), &request, sizeof(request));
    return buffer;
}

RequestInfo Communicator::parseRequest(const std::vector<char>& buffer, int bytesReceived) {
    std::cout << "Debug: Parsing request with " << bytesReceived << " bytes." << std::endl;
    RequestInfo request;

    request.id = static_cast<uint8_t>(buffer[0]);
    request.receivalTime = std::time(nullptr);
    request.buffer = std::vector<char>(buffer.begin() + 1, buffer.begin() + bytesReceived);

    std::cout << "Debug: Parsed request with ID: " << static_cast<int>(request.id)
        << " and data size: " << request.buffer.size() << std::endl;

    return request;
}

void Communicator::sendRequest(const RequestInfo& request) {
    std::cout << "Debug: Sending request with ID: " << static_cast<int>(request.id) << std::endl;
    int result;
    std::vector<char> data;
    data.push_back(static_cast<char>(request.id));

    data.insert(data.end(), request.buffer.begin(), request.buffer.end());

    result = send(m_socket, data.data(), data.size(), 0);

    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Failed to send request");
    }
    else {
        std::cout << "Debug: Request sent successfully." << std::endl;
    }
}

void Communicator::handleNewClient(SOCKET clientSocket) {
    std::cout << "Debug: New peer handler started for socket: " << clientSocket << std::endl;

    std::vector<char> buffer(1024);
    std::shared_ptr<IRequestHandler> handler = std::make_shared<RemoteControlingRequestHandler>();

    while (true) {
        std::cout << "Debug: Waiting to receive data from peer..." << std::endl;
        int bytesReceived = recv(clientSocket, buffer.data(), buffer.size(), 0);

        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recv failed or client disconnected. Error code: " << WSAGetLastError() << std::endl;
            break;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by peer." << std::endl;
            break;
        }

        std::cout << "Debug: Received " << bytesReceived << " bytes from peer." << std::endl;

        RequestInfo request = parseRequest(buffer, bytesReceived);
        IRequestHandler::RequestResult result = handler->handleRequest(request);

        if (!result.response.empty()) {
            std::cout << "Debug: Sending response to peer." << std::endl;
            int bytesSent = send(clientSocket, result.response.data(), result.response.size(), 0);
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
    closesocket(clientSocket);
}

