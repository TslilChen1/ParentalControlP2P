#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#include <string>
#include <vpx/vpx_encoder.h>
#include "json.hpp"

struct RTPPacket {
    uint8_t version;
    uint8_t padding;
    uint8_t extension;
    uint8_t csrcCount;
    uint8_t marker;
    uint8_t payloadType;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
    uint8_t* payload;
    size_t payloadSize;
};

class Comunication
{
private:
    SOCKET _sock;
    RTPPacket _rtpPacket;
public:
    //~Comunication();    
	SOCKET connectToOtherPeer(const std::string& role);
	void setRTPPacket(const RTPPacket& pkt);
    void sendRTPPacket();

    std::vector<char> rtpPacketToJson(const RTPPacket& packet);
    RTPPacket getRTPPacket();
};

