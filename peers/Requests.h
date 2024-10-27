#pragma once
#include <string>
#include <vector>

struct SDPRequest {

};

struct SDP
{
    long long sessionID;
    int sessionVersion;
    std::string candidateIP;
    int candidatePort;
    struct MediaCapabilities
    {
        std::vector<int> audioCodecs; 
        std::vector<int> videoCodecs;
    } mediaCapabilities;
    std::string transportInfo;
};

struct AuthenticationRequest
{
    int id;
    std::string username;
    std::string password;
    std::string role;
    std::string ip;
};