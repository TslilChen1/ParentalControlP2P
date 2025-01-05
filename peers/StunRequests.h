#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>


#include <ctime>
#include <cstdlib>
#include <sstream>
#include <string>

#define STUN_IP "74.125.197.127"
#pragma comment(lib, "Ws2_32.lib")

// Structure of a STUN message header
struct StunHeader {
    uint16_t message_type;      // Message type (e.g., 0x0001 for Binding Request)
    uint16_t message_length;    // Message length excluding the header
    uint32_t magic_cookie;      // Fixed value of 0x2112A442
    uint8_t transaction_id[12]; // Unique request identifier
};
static class StunRequests
{
private:
    static void generateRandomTransactionId(uint8_t transaction_id[12]);
    static StunHeader createStunRequest();
    //static void printStunResponse(const char* buffer, int length);
    static std::string parseStunAttributes(const char* buffer, int length);
public: static std::string runAngGetIP();
};
