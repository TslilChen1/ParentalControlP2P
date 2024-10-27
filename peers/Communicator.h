#pragma once

#include <winsock2.h>
#include <string>
#include <iostream>
#include "json.hpp" 
#include "Deserializer.h"
#include "Serializer.h"
#include <random>

class Communicator {
public:
    Communicator() = default;
    ~Communicator() = default;
    static void runAsPeer(const std::string& peer2IP, const SOCKET& connectionToSignalingServerSocket, const std::string& role);
    static void runAsParent(const std::string& peer2IP, const SOCKET& connectionToSignalingServerSocket);
    static void runAsChild(const SOCKET& connectionToSignalingServerSocket);
private:
    static void sendAuthentication(const SOCKET& sock, AuthenticationRequest& authRequest);
    //static void sendSDPMessage(const SOCKET& sock, SDP& sdp);
    static std::string generateRandomSecret(int length);
    //static std::string hashSHA256(const std::string& input);
    static std::string getLocalIPAddress();
};