#pragma once
#include <mutex>
#include <condition_variable>
#include <memory>
#include "RemoteControlingRequestHandler.h" 
#include <winsock2.h>

class Communicator {
public:
    Communicator();
    ~Communicator() = default;
    void runAsServer();
    void runAsClient(const std::string& serverAddress);
    void printMenu();

private:
    void bindAndListen(); 

    std::vector<char> Serialize(const BlockingWebRequest& request);
    std::vector<char> Serialize(const KeyBoardReadingRequest& request);

    void handleNewClient(SOCKET clientSocket);
    RequestInfo parseRequest(const std::vector<char>& buffer, int bytesReceived);
    void sendRequest(const RequestInfo& request);

    SOCKET m_socket;

    std::unique_ptr<RemoteControlingRequestHandler> m_handlerFactory;
    bool m_connectionEstablished;
    std::mutex m_connectionMutex;
    std::condition_variable m_connectionCv;
};
