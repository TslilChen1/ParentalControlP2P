#pragma once
#include "Requests.h"
#include "json.hpp"

class Serializer
{
public:

	static std::vector<char> serializeServerHello(ServerHello& request);
};

