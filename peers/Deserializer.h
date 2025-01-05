#pragma once
#include "Requests.h"
#include "Responses.h"

#include "json.hpp"


class Deserializer
{
public:
	const static SDP deserializeSDP(const std::vector<char>& buffer);
	const static AuthResponse deserializeAuthResponse(const std::vector<char>& buffer);

	const static ServerHello deserializeServerHello(const std::vector<char>& buffer);


};

