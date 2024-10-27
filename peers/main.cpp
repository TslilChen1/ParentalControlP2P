#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include "Communicator.h"
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[]) {
    try {
        SOCKET  connectionToSignalingServerSocket;
        /*if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <role>" << std::endl;
            std::cerr << "Example: " << argv[0] << " Parent" << std::endl;
            return 1;
        }*/

        Communicator communicator;

        //std::string role = argv[1];
        std::string role;
        std::cout << "Please enter your role (Parent/Child): ";
       
        do
        {
            std::getline(std::cin, role);
        }
		while (role != "Parent" && role != "Child");


       std::string serverAddress = "127.0.0.1";

        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed with error code: " << result << std::endl;
            return 1;
        }
        std::cout << "Debug: WSAStartup succeeded." << std::endl;

        connectionToSignalingServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connectionToSignalingServerSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed with error code: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }
        std::cout << "Debug: Socket created successfully." << std::endl;

        sockaddr_in serverInfo = { 0 };
        serverInfo.sin_family = AF_INET;
        serverInfo.sin_port = htons(8888);

        if (inet_pton(AF_INET, serverAddress.c_str(), &serverInfo.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << serverAddress << std::endl;
            closesocket(connectionToSignalingServerSocket);
            WSACleanup();
            return 1;
        }

        if (connect(connectionToSignalingServerSocket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
            std::cerr << "Connection failed with error code: " << WSAGetLastError() << std::endl;
            closesocket(connectionToSignalingServerSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Debug: Connected to server at " << serverAddress << ":" << 8888 << std::endl;

        if (role == "Parent" || role == "Child") {
            communicator.runAsPeer("127.0.0.1", connectionToSignalingServerSocket, role);
        }
        else {
            std::cerr << "Invalid role: " << role << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Main error: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
