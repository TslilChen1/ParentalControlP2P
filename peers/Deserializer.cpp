#include "Deserializer.h"

const SDP Deserializer::deserializeSDP(const std::vector<char>& buffer)
{
    nlohmann::json json_obj = nlohmann::json::parse(buffer.begin(), buffer.end());
    SDP sdp;

    sdp.sessionID = json_obj["sessionID"];
    sdp.sessionVersion = json_obj["sessionVersion"];
    sdp.candidateIP = json_obj["candidateIP"];
    sdp.candidatePort = json_obj["candidatePort"];
    sdp.mediaCapabilities.audioCodecs = json_obj["mediaCapabilities"]["audioCodecs"].get<std::vector<int>>();
    sdp.mediaCapabilities.videoCodecs = json_obj["mediaCapabilities"]["videoCodecs"].get<std::vector<int>>();
    sdp.transportInfo = json_obj["transportInfo"];

    return sdp;
}

const AuthResponse Deserializer::deserializeAuthResponse(const std::vector<char>& buffer)
{
    // Parse the received buffer as JSON
    nlohmann::json json_obj = nlohmann::json::parse(buffer.begin(), buffer.end());

    AuthResponse response;
    response.status = json_obj["status"];
    response.message = json_obj["message"];

    return response;
}
