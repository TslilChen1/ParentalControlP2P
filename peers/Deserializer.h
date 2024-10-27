#pragma once
#include "Requests.h"
#include <vector>
#include "json.hpp"
#include "Responses.h"

class Deserializer
{
public:
	const static SDP deserializeSDP(const std::vector<char>& buffer);
	const static AuthResponse deserializeAuthResponse(const std::vector<char>& buffer);
};

