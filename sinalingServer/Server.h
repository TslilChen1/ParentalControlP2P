#pragma once

// Server.h
#include <map>
#include "Communicator.h"
#include <thread>
#include "SQLiteWrapper.h"  // A custom wrapper class to interact with SQLite
#include <mutex>
#include "Desirialize.h"
#include "Serialize.h"
enum class State {
    authentication,
    Handle_connections,
    sdp,
    ice
};

struct Parent {
    std::shared_ptr<Communicator> parentCommunicator;
    std::vector<std::shared_ptr<Communicator>> childrenCommunicators;
    int roomId;
};

class Server {
public:
    Server(std::shared_ptr<SQLiteWrapper> database);
    ~Server();
    void start();

private:
    SOCKET serverSocket;


    std::map<std::string, Parent> waitingPeers;

    std::shared_ptr<SQLiteWrapper> database;
    //std::mutex clientMutex;

    void acceptClient();
    void handleAuthentication(const SOCKET& clientSocket);

    std::mutex peersMutex;  // Mutex for protecting access to waitingPeers

    int nextRoomId = 1; // Variable to track the next available room ID

    const void handleSDPOffer(const std::string& hashedSecret, const std::vector<char>& sdpOffer);
    const void handleSDPAnswer(const std::string& hashedSecret, const std::vector<char>& sdpAnswer);

};
