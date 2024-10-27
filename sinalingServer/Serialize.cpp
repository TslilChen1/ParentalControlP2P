#include "Serialize.h"


const std::vector<char> Serialize::serializeAuthResponse(const AuthResponse& response)
{
    // יצירת JSON מהמבנה של AuthResponse
    nlohmann::json j = nlohmann::json{
        {"status", response.status},
        {"message", response.message}
    };

    std::string json_str = j.dump();
    std::vector<char> char_vec(json_str.begin(), json_str.end());

    return std::move(char_vec); 
}
