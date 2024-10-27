#pragma once

// Communicator.h
#include <WinSock2.h>
#include <iostream>
#include "json.hpp" // JSON library
#include "Requests.h"
#include <chrono>        // For std::chrono::steady_clock and timing functions

#pragma comment(lib, "Ws2_32.lib")


class Communicator {
public:
    Communicator(const SOCKET& sock);
    ~Communicator();
    void sendMessage(const std::vector<char> message);
    std::vector<char> receiveMessage();

private:
    SOCKET clientSocket;
};
