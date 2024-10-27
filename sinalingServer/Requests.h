#pragma once
#include <string>

enum RequestType
{
	REGISTER_REQUEST = 0,
	LOGIN_REQUEST = 1,
	ERROR__TIMEOUT = 2
};


struct AuthenticationRequest
{
	int id;
	std::string username;
	std::string password;
	std::string role;
	std::string ip;
};