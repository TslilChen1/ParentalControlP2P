#pragma once
#include <string>

enum AuthStatus {
	SUCCESS_LOGIN = 0,
	SUCCESS_SIGNUP = 1,
	FAILED_SIGNUP = 2,
	USER_NOT_FOUND = 3,
	PASSWORD_INCORRECT = 4,
	USER_ALREADY_EXISTS = 5,
	UNKNOWN_ERROR = 6,
	SUCCESS_BIND = 7,
	FAILURE_BIND = 8
};

struct AuthResponse {
    int status;
	std::string message;
};
