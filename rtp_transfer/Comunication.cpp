#include "Comunication.h"
#include <iostream>
#include <vpx/vpx_encoder.h>
#include <vpx/vpx_codec.h>
#include <vpx/vp8dx.h>  
#include <vector>

#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")


/*Comunication::~Comunication()
{
    delete[] _rtpPacket.payload;
}*/

SOCKET Comunication::connectToOtherPeer(const std::string& role)
{
    WSADATA wsaData;
    SOCKET sock, clientSock;
    sockaddr_in serverAddr, clientAddr;
    int clientSize = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return INVALID_SOCKET;
    }


	if (role == "Child")
{        
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed!" << std::endl;
            WSACleanup();
            return INVALID_SOCKET;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);  
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  

        if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed!" << std::endl;
            closesocket(sock);
            WSACleanup();
            return INVALID_SOCKET;
        }
        this->_sock = sock;
        return sock;
	}
    else
    {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed!" << std::endl;
            WSACleanup();
            return INVALID_SOCKET;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);  
        serverAddr.sin_addr.s_addr = INADDR_ANY;  


        if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed!" << std::endl;
            closesocket(sock);  
            WSACleanup();
            return INVALID_SOCKET;
        }

        if (listen(sock, 1) == SOCKET_ERROR) {
            std::cerr << "Listen failed!" << std::endl;
            closesocket(sock);
            WSACleanup();
            return INVALID_SOCKET;
        }

        clientSock = accept(sock, (struct sockaddr*)&clientAddr, &clientSize);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "Accept failed!" << std::endl;
            closesocket(sock);
            WSACleanup();
            return INVALID_SOCKET;
        }
        this->_sock = clientSock;
        return clientSock;
    }
}

void Comunication::setRTPPacket(const RTPPacket& pkt)
{
    _rtpPacket = pkt;
}

void Comunication::sendRTPPacket()
{
    std::cout << "Sending RTP Packet, Payload Size: " << _rtpPacket.payloadSize << " bytes" << std::endl;
    send(_sock, rtpPacketToJson(_rtpPacket).data(), rtpPacketToJson(_rtpPacket).size(), 0);
}


std::vector<char> Comunication::rtpPacketToJson(const RTPPacket& packet)
{
    nlohmann::json j;

    j["version"] = packet.version;
    j["padding"] = packet.padding;
    j["extension"] = packet.extension;
    j["csrcCount"] = packet.csrcCount;
    j["marker"] = packet.marker;
    j["payloadType"] = packet.payloadType;
    j["sequenceNumber"] = packet.sequenceNumber;
    j["timestamp"] = packet.timestamp;
    j["ssrc"] = packet.ssrc;

    std::vector<uint8_t> payloadVec(packet.payload, packet.payload + packet.payloadSize);
    j["payload"] = payloadVec;

    j["payloadSize"] = packet.payloadSize;

    std::string content = j.dump();
    std::vector <char> buffer(content.size());
    std::copy(content.begin(), content.end(), buffer.begin());
    return buffer;
}

RTPPacket Comunication::getRTPPacket()
{
    std::vector<char> buffer;

    try {
        buffer.resize(500000); 
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Error allocating memory: " << e.what() << std::endl;
    }

   recv(_sock,buffer.data(), buffer.size(), 0);
   nlohmann::json j = nlohmann::json::parse(buffer.begin(), buffer.end());


    RTPPacket rtpPacket;
    rtpPacket.version = j["version"];
    rtpPacket.padding = j["padding"];
    rtpPacket.extension = j["extension"];
    rtpPacket.csrcCount = j["csrcCount"];
    rtpPacket.marker = j["marker"];
    rtpPacket.payloadType = j["payloadType"];
    rtpPacket.sequenceNumber = j["sequenceNumber"];
    rtpPacket.timestamp = j["timestamp"];
    rtpPacket.ssrc = j["ssrc"];

    std::vector<uint8_t> payloadVec = j["payload"].get<std::vector<uint8_t>>();
    rtpPacket.payloadSize = payloadVec.size();
    rtpPacket.payload = new uint8_t[payloadVec.size()];
    std::memcpy(rtpPacket.payload, payloadVec.data(), payloadVec.size());

    return rtpPacket;
}

