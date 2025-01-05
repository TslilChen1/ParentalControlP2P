#include "Serializer.h"

std::vector<char> Serializer::serializeServerHello(ServerHello& request)
{
    nlohmann::json json_obj;

    json_obj["ProtocolVersion"] = request.ProtocolVersion;
    json_obj["random"] = request.random;
    json_obj["session_id"] = request.session_id;
    json_obj["cipher_suite"] = request.cipher_suite;

    std::string json_str = json_obj.dump();
    return std::vector<char>(json_str.begin(), json_str.end());
}
