#include "Communicator.h"


// Run Ngrok in a separate thread to avoid blocking the server
//std::thread ngrokThread(&Communicator::startServeo, this);
//ngrokThread.detach();  // Run Ngrok in the background

/*void Communicator::startServeo()
{
    int ret = std::system("ssh -R 80:localhost:9315 serveo.net");
    if (ret != 0) {
        std::cerr << "Failed to run Serveo command. Return code: " << ret << std::endl;
    }
    else {
        std::cout << "Serveo command executed successfully." << std::endl;
    }
}*/
// Communicator.cpp


Communicator::Communicator(const SOCKET& sock) : clientSocket(sock) {}

Communicator::~Communicator() {
    closesocket(clientSocket);
}

void Communicator::sendMessage(const std::vector<char> message) {
    send(clientSocket, message.data(), message.size(), 0);
}

std::vector<char> Communicator::receiveMessage() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);

    // Set a 60-second timeout
    struct timeval timeout;
    timeout.tv_sec = 45;
    timeout.tv_usec = 0;

    // Wait for the socket to be ready to read (with a timeout)
    int selectResult = select(clientSocket + 1, &readfds, nullptr, nullptr, &timeout);

    if (selectResult == -1) {
        std::cerr << "Error in select(): " << WSAGetLastError() << std::endl;
        return {};  // Return empty vector on error
    }
    else if (selectResult == 0) {
        std::cerr << "Warning: No data received after 60 seconds (timeout)." << std::endl;
        return { ERROR__TIMEOUT };  // Return vector with timeout error code only
    }

    // If data is available, proceed to receive it
    std::vector<char> buffer(1024);  // Allocate buffer of size 1024
    int bytesReceived = recv(clientSocket, buffer.data(), buffer.size(), 0);

    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error receiving message: " << WSAGetLastError() << std::endl;
        return {};  // Return empty vector if an error occurs
    }

    // Resize buffer to the actual number of bytes received
    //buffer.resize(bytesReceived);
    return buffer;
}

SOCKET Communicator::getsocket()
{
    return this->clientSocket;
}
