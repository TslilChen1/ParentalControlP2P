#pragma once

#include <string>
#include "sqlite3.h"
#include <vector>
#include <iostream>
#include <memory>
#include "Requests.h"


class SQLiteWrapper {
public:
    SQLiteWrapper();
    ~SQLiteWrapper();

    //bool authenticateUser(AuthenticationRequest& details);
    
    int userExists(AuthenticationRequest& details);
    bool addUser(AuthenticationRequest& details);
    std::string getMatchingPeer(const std::string& username,const std::string & password, const std::string& role);
    //std::string getMatchingIP(const std::string& username, const std::string& excludeIP);

    void saveSDPOffer(const std::string& username, const std::vector<char>& sdpOffer);
    std::vector<char> getSDPResponse(const std::string& username);
    void saveSDPResponse(const std::string& username, const std::vector<char>& sdpResponse);


    std::vector<char> getSDPOffer(const std::string& username);
    
    //bool checkCredentialsOfUsers(const std::string& username, const std::string& password);
    bool setRoomID(std::string username, std::string password, int roomID);

private:
    sqlite3* db;
    bool executeQuery(const std::string& query);
};

