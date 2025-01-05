#include <iostream>
#include "Encoder.h"
#include "Decoder.h"
#include <iostream>
#include "Comunication.h"
#include "ScreenCapture.h"


#include "LiveImageDisplay.h"  // New header for live display
#include "LockImage.h"


void printRTPPacket(const RTPPacket& packet) {
    std::cout << "RTP Packet Information:" << std::endl;

    std::cout << "Version: " << static_cast<int>(packet.version) << std::endl;
    std::cout << "Padding: " << static_cast<int>(packet.padding) << std::endl;
    std::cout << "Extension: " << static_cast<int>(packet.extension) << std::endl;
    std::cout << "CSRC Count: " << static_cast<int>(packet.csrcCount) << std::endl;
    std::cout << "Marker: " << static_cast<int>(packet.marker) << std::endl;
    std::cout << "Payload Type: " << static_cast<int>(packet.payloadType) << std::endl;
    std::cout << "Sequence Number: " << packet.sequenceNumber << std::endl;
    std::cout << "Timestamp: " << packet.timestamp << std::endl;
    std::cout << "SSRC: " << packet.ssrc << std::endl;

    std::cout << "Payload Size: " << packet.payloadSize << std::endl;
    std::cout << "Payload (Hex): ";

    for (size_t i = 0; i < packet.payloadSize; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(packet.payload[i]) << " ";
    }
    std::cout << std::dec << std::endl;  
}


// עובדדדדדד
int main()
{
    std::cout << "whats your role :";
    std::string role = "";
    std::cin >> role;

    Comunication c;
    SOCKET s = c.connectToOtherPeer(role);

    if (role == "Child")
    {
        Sleep(100);  // Sleeps for 1 second

        ScreenCapture::gdiscreen();
        RTPPacket pkg = Encoder::encodeImage("screen.png");
        sizeof(pkg);
        printRTPPacket(pkg);

        c.setRTPPacket(pkg);
        c.sendRTPPacket();
        
        std::cin.get();
        std::cin.get();
        std::cin.get();
    }
    else
    {
        LiveImageDisplay oneFrame;
        RTPPacket pkg = c.getRTPPacket();
        //printRTPPacket(pkg);
        Decoder::decodeImage(pkg, "output.png");
        oneFrame.showOneFrame(L"output.png");

        std::cin.get();
        std::cin.get();
        std::cin.get();
    }
}