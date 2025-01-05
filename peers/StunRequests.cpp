#include "StunRequests.h"


// Function to generate a random transaction ID
void StunRequests::generateRandomTransactionId(uint8_t transaction_id[12])
{
    srand(static_cast<unsigned int>(time(0))); // Initialize the generator
    for (int i = 0; i < 12; ++i) {
        transaction_id[i] = static_cast<uint8_t>(rand() % 256);
    }
}


// Function to create a STUN request
StunHeader StunRequests::createStunRequest()
{
    StunHeader request;
    request.message_type = htons(0x0001);  // Binding Request in network byte order
    request.message_length = htons(0x0000); // No attributes at this stage
    request.magic_cookie = htonl(0x2112A442);
    generateRandomTransactionId(request.transaction_id);
    return request;
}

/*
void StunRequests::printStunResponse(const char* buffer, int length)
{
    // Print the message type
    uint16_t message_type = ntohs(*reinterpret_cast<const uint16_t*>(buffer));
    std::cout << "Message Type: 0x" << std::hex << message_type << std::dec << "\n";

    // Print the message length
    uint16_t message_length = ntohs(*reinterpret_cast<const uint16_t*>(buffer + 2));
    std::cout << "Message Length: " << message_length << "\n";

    // Print the magic cookie
    uint32_t magic_cookie = ntohl(*reinterpret_cast<const uint32_t*>(buffer + 4));
    std::cout << "Magic Cookie: 0x" << std::hex << magic_cookie << std::dec << "\n";

    // Print the transaction ID
    std::cout << "Transaction ID: ";
    for (int i = 0; i < 12; ++i) {
        std::cout << std::hex << static_cast<int>(static_cast<unsigned char>(buffer[8 + i])) << " ";
    }
    std::cout << std::dec << "\n";

    // Additional handling could go here to print any attributes in the response
}
*/

std::string StunRequests::parseStunAttributes(const char* buffer, int length)
{
    const uint32_t magic_cookie = 0x2112A442;
    int pos = 20; // Starting position after the STUN header

    while (pos < length) {
        // Read attribute type and length
        uint16_t attribute_type = ntohs(*reinterpret_cast<const uint16_t*>(buffer + pos));
        uint16_t attribute_length = ntohs(*reinterpret_cast<const uint16_t*>(buffer + pos + 2));

        // Check for XOR-Mapped Address attribute
        if (attribute_type == 0x0020) {
            uint8_t family = *(buffer + pos + 5);

            if (family == 0x01) { // IPv4 only
                uint16_t xor_port = ntohs(*reinterpret_cast<const uint16_t*>(buffer + pos + 6)) ^ (magic_cookie >> 16);
                uint32_t xor_ip = ntohl(*reinterpret_cast<const uint32_t*>(buffer + pos + 8)) ^ magic_cookie;

                // Construct the IP address string
                std::string ip_address = std::to_string((xor_ip >> 24) & 0xFF) + "." +
                    std::to_string((xor_ip >> 16) & 0xFF) + "." +
                    std::to_string((xor_ip >> 8) & 0xFF) + "." +
                    std::to_string(xor_ip & 0xFF) +
                    ":" + std::to_string(xor_port);

                // Return the result
                return ip_address;
            }
            else {
                // If the family is not IPv4, return an error message
                return "Family not supported (expected IPv4).";
            }
        }

        // Move to the next attribute
        pos += 4 + attribute_length;
    }

    return "XOR-Mapped Address attribute not found in response.";
}

std::string StunRequests::runAngGetIP()
{
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;
    int serverAddrLen = sizeof(serverAddr);
    char buffer[512];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return nullptr;
    }

    // Create a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return nullptr;
    }

    // Set a timeout for receiving a response (5 seconds)
    int timeout = 5000; // 5000 milliseconds = 5 seconds
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "Failed to set socket timeout.\n";
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    // Define the server address (e.g., Google's STUN server)
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3478);  // Default STUN port
    if (inet_pton(AF_INET, STUN_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported.\n";
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    // Create a STUN request
    StunHeader request = createStunRequest();

    // Send the request
    if (sendto(sock, reinterpret_cast<char*>(&request), sizeof(request), 0,
        (struct sockaddr*)&serverAddr, serverAddrLen) == SOCKET_ERROR) {
        std::cerr << "Failed to send STUN request. Error: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }
    std::cout << "STUN request sent successfully.\n";

    // Receive the response
    int responseLen = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
    if (responseLen == SOCKET_ERROR) {
        std::cerr << "Failed to receive STUN response. Error: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    // Print response details
    std::cout << "STUN response received, length: " << responseLen << " bytes.\n";
    //printStunResponse(buffer, responseLen);
    std::string details = parseStunAttributes(buffer, responseLen);
    std::string ip, port;

    std::istringstream iss(details);
    std::getline(iss, ip, ':');  
    std::getline(iss, port);    

    std::cout << "IP: " << ip << std::endl;
    std::cout << "Port: " << port << std::endl;

    // Cleanup
    closesocket(sock);
    WSACleanup();
    return ip;
}

