/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 _________  ________  ___       ___  ___                     ___    ___ ________  ___  ________
|\___   ___\\   ____\|\  \     |\  \|\  \                   |\  \  /  /|\   __  \|\  \|\   __  \
\|___ \  \_\ \  \___|\ \  \    \ \  \ \  \      ____________\ \  \/  / | \  \|\  \ \  \ \  \|\  \
     \ \  \ \ \_____  \ \  \    \ \  \ \  \    |\____________\ \    / / \ \   __  \ \  \ \   _  _\
      \ \  \ \|____|\  \ \  \____\ \  \ \  \___\|____________|\/  /  /   \ \  \ \  \ \  \ \  \\  \|
       \ \__\  ____\_\  \ \_______\ \__\ \_______\          __/  / /      \ \__\ \__\ \__\ \__\\ _\
        \|__| |\_________\|_______|\|__|\|_______|         |\___/ /        \|__|\|__|\|__|\|__|\|__|
              \|_________|                                 \|___|/


                 ________  ________  ________  _______   ________   _________  ________  ___
                |\   __  \|\   __  \|\   __  \|\  ___ \ |\   ___  \|\___   ___\\   __  \|\  \
                \ \  \|\  \ \  \|\  \ \  \|\  \ \   __/|\ \  \\ \  \|___ \  \_\ \  \|\  \ \  \
                 \ \   ____\ \   __  \ \   _  _\ \  \_|/_\ \  \\ \  \   \ \  \ \ \   __  \ \  \
                  \ \  \___|\ \  \ \  \ \  \\  \\ \  \_|\ \ \  \\ \  \   \ \  \ \ \  \ \  \ \  \____
                   \ \__\    \ \__\ \__\ \__\\ _\\ \_______\ \__\\ \__\   \ \__\ \ \__\ \__\ \_______\
                    \|__|     \|__|\|__|\|__|\|__|\|_______|\|__| \|__|    \|__|  \|__|\|__|\|_______|

                                                     ____________
                                                    |\____________\
                                                    \|____________|

                          ________  ________  ________   _________  ________  ________  ___
                         |\   ____\|\   __  \|\   ___  \|\___   ___\\   __  \|\   __  \|\  \
                         \ \  \___|\ \  \|\  \ \  \\ \  \|___ \  \_\ \  \|\  \ \  \|\  \ \  \
                          \ \  \    \ \  \\\  \ \  \\ \  \   \ \  \ \ \   _  _\ \  \\\  \ \  \
                           \ \  \____\ \  \\\  \ \  \\ \  \   \ \  \ \ \  \\  \\ \  \\\  \ \  \____
                            \ \_______\ \_______\ \__\\ \__\   \ \__\ \ \__\\ _\\ \_______\ \_______\
                             \|_______|\|_______|\|__| \|__|    \|__|  \|__|\|__|\|_______|\|_______|


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
#include "Communicator.h"
#include <iostream>
#include <thread>



#pragma comment(lib, "Ws2_32.lib")


#define PORT 8888

int main(int argc, char* argv[]) {
    /*if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <role>" << std::endl;
        std::cerr << "Example: " << argv[0] << " Parent" << std::endl;
        return 1;
    }

    std::string role = argv[1];
    if (role != "Parent" && role != "Child") {
        std::cerr << "Invalid role: " << role << ". Role must be either 'Parent' or 'Child'." << std::endl;
        return 1;
    }
    */
    try {
        
        SOCKET  connectionToSignalingServerSocket;
        Communicator communicator;

        
        
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
        serverInfo.sin_port = htons(PORT);

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

        std::cout << "Debug: Connected to server at " << serverAddress << ":" << PORT << std::endl;

        if (role == "Parent" || role == "Child") {
            communicator.runAsPeer(connectionToSignalingServerSocket, role);
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
