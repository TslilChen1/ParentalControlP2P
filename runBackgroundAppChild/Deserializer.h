#pragma once
#include "Requests.h"
#include "Responses.h"
#include "json.hpp"


class Deserializer
{
public:

	const static ClientHello deserializeClientHello(const std::vector<char>& buffer);
	const static Certificate deserializeCertificate(const std::vector<char>& buffer);

	const static KeyBoardReadingRequest deserializeKeyBoardReadingRequest(const std::vector<char>& buffer);
	const static  BlockingWebRequest deserializeBlockingWebRequest(const std::vector<char>& buffer);


};

