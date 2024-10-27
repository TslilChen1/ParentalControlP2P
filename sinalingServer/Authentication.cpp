#include "Authentication.h"

// Constructor that accepts an AuthenticationRequest
Authentication::Authentication(const AuthenticationRequest& request, std::shared_ptr<SQLiteWrapper> database)
    : authRequest(request) , db(database) 
{}

void Authentication::login(AuthResponse& authResponse)
{
    // Check if the user already exists
    switch (db->userExists(authRequest))
    {
    case USER_ALREADY_EXISTS:
        authResponse.status = SUCCESS_LOGIN;
        authResponse.message = "login successful";
        break;
    case USER_NOT_FOUND:
		authResponse.status = USER_NOT_FOUND;
        authResponse.message = "User not found";
        break;
    case PASSWORD_INCORRECT:
		authResponse.status = PASSWORD_INCORRECT;
		authResponse.message = "Incorrect password";
        break;
    }
}

void Authentication::signUp(AuthResponse& authResponse)
{
    // IN THE 2 CASES THE USER IS EXISTS
    if (db->userExists(authRequest) == USER_ALREADY_EXISTS || db->userExists(authRequest) == PASSWORD_INCORRECT)
    {
        authResponse.status = USER_ALREADY_EXISTS; // User already exists.
		authResponse.message = "Username already exists! choose different one.";
    }
    else
    {
        if (db->addUser(authRequest))
        {
            authResponse.status = SUCCESS_SIGNUP;
			authResponse.message = "signup successful";
        }
        else {
            authResponse.status = FAILED_SIGNUP;
			authResponse.message = "signup failed";
        }

    }
}

AuthResponse Authentication::authenticateUser()
{
    AuthResponse authResponse;

    if (this->authRequest.id == REGISTER_REQUEST) signUp(authResponse);
    else if (this->authRequest.id == LOGIN_REQUEST) login(authResponse);

	return authResponse;

}



