#pragma once

#include "json.hpp"
#include "Requests.h"
#include "Responses.h"
#include "SQLiteWrapper.h"


// Authentication class
class Authentication {
public:
    // Constructor that accepts an AuthenticationRequest
    Authentication(const AuthenticationRequest& request, std::shared_ptr<SQLiteWrapper> db);

    // Method to handle the authentication process
    AuthResponse authenticateUser();
    void login(AuthResponse& authResponse);
    void signUp(AuthResponse& authResponse);

private:
    AuthenticationRequest authRequest;  // Holds the authentication details
    std::shared_ptr<SQLiteWrapper> db;  // Pointer to the SQLiteWrapper instance
};

