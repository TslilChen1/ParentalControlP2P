#pragma once
#include <vector>
#include "Responses.h"
#include "json.hpp"

class Serialize
{
public:
	const static std::vector<char> serializeAuthResponse(const AuthResponse& response);
};

