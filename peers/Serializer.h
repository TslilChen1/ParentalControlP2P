#pragma once
#include "Requests.h"
#include "json.hpp"

class Serializer
{
public:
	static std::vector<char> serializeSDP(SDP& request);

	 
};

