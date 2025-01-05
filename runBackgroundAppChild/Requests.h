#pragma once
#include <string>
#include <vector>

struct ClientHello {
    std::string ProtocolVersion; // - Tslil and Yair DTLS
    uint64_t random;
    uint32_t session_id;
    uint16_t cipher_suites_length;
    std::vector<uint16_t> cipher_suites; // examples:0xC013 = TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 / 0x0035 = TLS_RSA_WITH_AES_128_CBC_SHA256
};

struct ServerHello {
    std::string ProtocolVersion; // - Tslil and Yair DTLS
    uint64_t random;
    uint32_t session_id;
    uint16_t cipher_suite;
};

struct Certificate
{
    std::string name; // (CA)
    std::string departure_date;
    std::string expiration_date;
    std::pair<uint64_t, uint64_t> publickey; // (e, n);
    uint64_t digital_signature;
};


enum ParentResquests
{
    TURN_OFF_COMPUTER_REQUEST = 1,
    KEY_BOARD_READING_REQUEST = 2,
    KEY_BOARD_CONTROLING_REQUEST = 3,
    BLOCK_WEB_REQUEST = 4
};

struct KeyBoardReadingRequest
{
    int timeInSecondToRead;
};

struct KeyBoardControlingRequest
{
    char ch;
    int timeInSecondToControl;
};

struct BlockingWebRequest
{
    std::string webName;
};