#include "Serializer.h"

std::vector<char> Serializer::serializeSDP(SDP& request)
{
    nlohmann::json jsonSDP = {
       {"sessionID", request.sessionID},
       {"sessionVersion", request.sessionVersion},
       {"candidateIP", request.candidateIP},
       {"candidatePort", request.candidatePort},
       {"mediaCapabilities", {
           {"audioCodecs", request.mediaCapabilities.audioCodecs},
           {"videoCodecs", request.mediaCapabilities.videoCodecs}
       }},
       {"transportInfo", request.transportInfo}
    };
    std::string content = jsonSDP.dump();
    std::vector <char> buffer(content.size());
	std::copy(content.begin(), content.end(), buffer.begin());
    return buffer;
}


std::vector<char> Serializer::serializeClientHello(const ClientHello& request)
{
    nlohmann::json json_obj;

    json_obj["ProtocolVersion"] = request.ProtocolVersion;
    json_obj["random"] = request.random;
    json_obj["session_id"] = request.session_id;
    json_obj["cipher_suites_length"] = request.cipher_suites_length;
    json_obj["cipher_suites"] = request.cipher_suites;

    std::string json_str = json_obj.dump();
    return std::vector<char>(json_str.begin(), json_str.end());
}
std::vector<char> Serializer::serializeCertificate(Certificate& request)
{
    nlohmann::json json_obj;

    json_obj["name"] = request.name;
    json_obj["Departure_date"] = request.departure_date;
    json_obj["expiration_date"] = request.expiration_date;

    json_obj["publickey"]["n"] = request.publickey.first;
    json_obj["publickey"]["e"] = request.publickey.second;

    json_obj["Digital_signature"] = request.digital_signature;

    std::string json_str = json_obj.dump();
    return std::vector<char>(json_str.begin(), json_str.end());
}

std::vector<char> Serializer::Serialize(const KeyBoardReadingRequest& request)
{
    std::vector<char> buffer(sizeof(request));
    std::memcpy(buffer.data(), &request, sizeof(request));
    return buffer;
}

std::vector<char> Serializer::Serialize(const std::string& webName)
{
    return std::vector<char>(webName.begin(), webName.end());
}
