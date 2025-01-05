#pragma once
#include <string>
#include <WinSock2.h>
#include "Requests.h"
#include "DTLS.h"
#include "IRequestHandler.h"

class DTLS;  // Forward declaration

class Communicator
{
private:
    SOCKET _sock;
    RequestInfo parseRequest(const std::vector<char>& buffer, int bytesReceived, const DTLS& dtls);


public:
    bool connectToOtherPeer(int port);
    bool sendMessage(std::pair<uint64_t, uint64_t> publicKey);
    uint64_t receive_msg_uint64_t();
    std::pair<uint64_t, uint64_t> receive_msg_publicKey();
    bool sendMessage(ServerHello& msg);
    bool sendMessage(uint64_t msg);
    std::vector<char> receiveMessage();
    bool sendBytes(std::vector<char>& buffer);
    bool runChildMain(DTLS& dtls);

};

