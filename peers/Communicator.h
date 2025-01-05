#pragma once
#include "StunRequests.h"
#include "Serializer.h"
#include "SHA256.h"

#include <string>
#include "json.hpp" 
#include <random>

class DTLS;  // Forward declaration

struct RequestInfo
{
    uint8_t id; // RequestId as a single byte   
    time_t receivalTime;
    std::vector<char> buffer;
};

class Communicator {

private:

    SOCKET _sock; // the p2p socket
    void sendAuthentication(const SOCKET& sock, AuthenticationRequest& authRequest);
    //static void sendSDPMessage(const SOCKET& sock, SDP& sdp);
    static std::string generateRandomSecret(int length);
    //static std::string hashSHA256(const std::string& input);
    static std::string getLocalIPAddress();

    bool connectToOtherPeer(const std::string& role);
    bool sendRequest(const RequestInfo& request, const DTLS& dtls);

public:
    Communicator() = default;
    ~Communicator();
    void runAsPeer(const SOCKET& connectionToSignalingServerSocket, const std::string& role);
    void runAsParent(const SOCKET& connectionToSignalingServerSocket);
    void runAsChild(const SOCKET& connectionToSignalingServerSocket);

    bool sendBytes(std::vector<char>& buffer);

    // for dtls
    bool sendMessage(uint64_t msg);
    bool sendMessage(std::pair<uint64_t, uint64_t> publicKey);
    uint64_t receive_msg_uint64_t();
    std::pair<uint64_t, uint64_t> receive_msg_publicKey();

    bool sendMessage(ClientHello& msg);
    bool sendMessage(Certificate& msg);


    std::vector<char> receiveMessage();

    void printMenu();
};