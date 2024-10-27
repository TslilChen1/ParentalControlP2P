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
