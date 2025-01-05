#pragma once
#include "Requests.h"
#include "json.hpp"

class Serializer
{
public:
	// func to autication
	static std::vector<char> serializeSDP(SDP& request);

	// funcs for encryption
	static std::vector<char> serializeClientHello(const ClientHello& request);
	static std::vector<char> serializeCertificate(Certificate& request);


	// funcs for commands in app
	static std::vector<char> Serialize(const KeyBoardReadingRequest& request);
	static std::vector<char> Serialize(const std::string& webName);
};

