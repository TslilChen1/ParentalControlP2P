#include "Deserializer.h"
#include <iostream>

const ClientHello Deserializer::deserializeClientHello(const std::vector<char>& buffer)
{
    nlohmann::json json_obj = nlohmann::json::parse(buffer.begin(), buffer.end());

    ClientHello clientHello;

    clientHello.ProtocolVersion = json_obj["ProtocolVersion"];

    clientHello.random = json_obj["random"];

    clientHello.session_id = json_obj["session_id"];
    clientHello.cipher_suites_length = json_obj["cipher_suites_length"];

    clientHello.cipher_suites = json_obj["cipher_suites"].get<std::vector<uint16_t>>();

    return clientHello;
}

const Certificate Deserializer::deserializeCertificate(const std::vector<char>& buffer)
{
    nlohmann::json json_obj = nlohmann::json::parse(buffer.begin(), buffer.end());
    Certificate certificate;

    certificate.name = json_obj["name"];
    certificate.departure_date = json_obj["Departure_date"];
    certificate.expiration_date = json_obj["expiration_date"];

    certificate.publickey.first = json_obj["publickey"]["n"];
    certificate.publickey.second = json_obj["publickey"]["e"];

    certificate.digital_signature = json_obj["Digital_signature"];

    return certificate;
}

const KeyBoardReadingRequest Deserializer::deserializeKeyBoardReadingRequest(const std::vector<char>& buffer)
{
    KeyBoardReadingRequest request;
    std::memcpy(&request.timeInSecondToRead, buffer.data(), sizeof(request.timeInSecondToRead));
    return request;
}

const BlockingWebRequest Deserializer::deserializeBlockingWebRequest(const std::vector<char>& buffer)
{
    BlockingWebRequest request;
    request.webName = std::string(buffer.begin(), buffer.end());
    std::cout << "Desirilzer webName: " << request.webName << std::endl;

    return request;
}
