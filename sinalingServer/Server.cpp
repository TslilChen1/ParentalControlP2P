// Server.cpp
#include "Server.h"
#include "Authentication.h"

Server::Server(std::shared_ptr<SQLiteWrapper> database)
{
    this->database = database;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);
}

Server::~Server() {
    closesocket(serverSocket);
    WSACleanup();
}

void Server::start() {
    std::cout << "Server is listening on port 8888...\n";
    while (true) {
        acceptClient();
    }
}

void Server::acceptClient() {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

    std::thread([this, clientSocket]() {
        handleAuthentication(clientSocket);
        }).detach();
}

const void Server::handleSDPOffer(const std::string& hashedSecret, const std::vector<char>& sdpOffer)
{
    std::lock_guard<std::mutex> lock(peersMutex); // Locking the mutex

    auto it = waitingPeers.find(hashedSecret);
    if (it != waitingPeers.end()) {
        // Send the SDP offer to each child connected to the parent
        for (const auto& childComm : it->second.childrenCommunicators) {
            childComm->sendMessage(sdpOffer); // Send the SDP offer to each child
            std::cout << "Forwarded SDP offer to child." << std::endl;
        }
    }
}

const void Server::handleSDPAnswer(const std::string& hashedSecret, const std::vector<char>& sdpAnswer)
{
    std::lock_guard<std::mutex> lock(peersMutex); // Locking the mutex
    auto it = waitingPeers.find(hashedSecret);
    it->second.parentCommunicator->sendMessage(sdpAnswer);
	std::cout << "Forwarded SDP answer to parent. " << std::string(sdpAnswer.begin(), sdpAnswer.end()) << std::endl;
}

void Server::handleAuthentication(const SOCKET& clientSocket)
{
    std::unique_lock<std::mutex> lock(peersMutex);
    lock.unlock();


    std::shared_ptr<Communicator> comm = std::make_shared<Communicator>(clientSocket);
    std::vector<char> buffer(1024);
    AuthResponse authenticationResponse;
    AuthenticationRequest authenticationRequest;

    std::cout << "Waiting for client authentication..." << std::endl;

    std::vector<char> receivedMsg;
    std::string hashedSecret;
    char numberOfKids = '0';
    Parent newParent;
    do
    {
        do
        {
            receivedMsg = comm->receiveMessage();
        }
        while (receivedMsg[0] == ERROR__TIMEOUT && (receivedMsg[0] != SUCCESS_LOGIN || receivedMsg[0] != SUCCESS_SIGNUP));
        
        std::copy(receivedMsg.begin(), receivedMsg.end(), buffer.begin());
        nlohmann::json json_obj = nlohmann::json::parse(buffer.begin(), buffer.end());

       
        authenticationRequest = Desirialize::deserializeAuthentication(json_obj);
        std::cout << "Deserialized authentication request for user: " << authenticationRequest.username << std::endl;

        Authentication authentication(authenticationRequest, database);
        authenticationResponse = authentication.authenticateUser();

        comm->sendMessage(Serialize::serializeAuthResponse(authenticationResponse));


        if (authenticationResponse.status == SUCCESS_SIGNUP) {
            receivedMsg = comm->receiveMessage();
            if (authenticationRequest.role == "Parent")
            {
                numberOfKids = receivedMsg[0];
                hashedSecret.assign(receivedMsg.begin() + 1, receivedMsg.end());
                hashedSecret.erase(hashedSecret.find_last_not_of(" \n\r\t") + 1);
                std::cout << "Received hashed secret length: " << hashedSecret.length() << std::endl;
            }
            else
            {
                hashedSecret.assign(receivedMsg.begin() + 1, receivedMsg.end());
            }
            std::cout << "Received hashed secret: " << hashedSecret << std::endl;


            // Debugging: print all entries in the map
            std::cout << "Current waitingPeers map contents:" << std::endl;

            lock.lock();

            for (const auto& peer : waitingPeers) {
                std::cout << "Secret: " << peer.first << ", Room ID: " << peer.second.roomId
                    << ", Parent connected: " << (peer.second.parentCommunicator ? "Yes" : "No")
                    << ", Number of children connected: " << peer.second.childrenCommunicators.size() << std::endl;
            }

            lock.unlock();

            // Handle parent connections
            if (authenticationRequest.role == "Parent") {
                newParent.parentCommunicator = comm;  // Store parent communicator
                newParent.roomId = this->nextRoomId;
                this->nextRoomId++;
           
                lock.lock();
                waitingPeers[hashedSecret] = newParent;  // Add to the map
				lock.unlock();

                //hashedSecret.pop_back();  // Safely removes the last character
                
                std::cout << "Parent connected with secret: " << hashedSecret << " and assigned room ID: " << newParent.roomId << std::endl;
            }
            // Handle child connections
            else if (authenticationRequest.role == "Child")
            {
                auto startTime = std::chrono::steady_clock::now();
                auto timeout = std::chrono::seconds(30); // Timeout after 30 seconds 

                lock.lock();
                std::map<std::string, Parent>::iterator it;
                do
                {
                    it = waitingPeers.find(hashedSecret); // Find the secret in the map
                }
				while (it == waitingPeers.end() && std::chrono::steady_clock::now() - startTime < timeout);

                // Now check if a match was found
                if (it != waitingPeers.end()) {
                    it->second.childrenCommunicators.push_back(comm);  // Add child communicator to the list
                    std::cout << "Child connected with secret: " << hashedSecret << std::endl;
                }
                else {
                    std::cout << "No parent found for secret: " << hashedSecret << std::endl;
                }

				lock.unlock();

                std::cout << "Current waitingPeers map contents: CHILDRENNNNN" << std::endl;
                for (const auto& peer : waitingPeers) {
                    std::cout << "Secret: " << peer.first << ", Room ID: " << peer.second.roomId
                        << ", Parent connected: " << (peer.second.parentCommunicator ? "Yes" : "No")
                        << ", Number of children connected: " << peer.second.childrenCommunicators.size() << std::endl;
                }


            }
              
        }
    } while (authenticationResponse.status != SUCCESS_SIGNUP && authenticationResponse.status != SUCCESS_LOGIN);

    if (authenticationRequest.role == "Parent")
    {
        auto startTime = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds(45); // max 1 min

        //lock.lock();
        //auto& children = waitingPeers[hashedSecret].childrenCommunicators; // מתייחס ישירות לרשימה של ילדים

        //while (children.size() < (numberOfKids - '0') && (std::chrono::steady_clock::now() - startTime < timeout)) {
        while (std::chrono::steady_clock::now() - startTime < timeout) {};
        std::cout << "sizeeeeeeeeeeee: " << waitingPeers[hashedSecret].childrenCommunicators.size() << std::endl;
        //    std::cout << "Current number of connected children: " << children.size() << std::endl;
        //    lock.unlock();
        //    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Avoid busy-waiting
        //    lock.lock(); //
        //}

        //if (std::chrono::steady_clock::now() - startTime >= timeout) {
        //    std::cout << "Timeout reached waiting for child connections." << std::endl;
        //}
    }

    Parent parent = waitingPeers[hashedSecret];
    bool success = database->setRoomID(authenticationRequest.username, authenticationRequest.password, parent.roomId);


    AuthResponse bindResponse;
    if (success) {
        bindResponse.status = SUCCESS_BIND;
        std::cout << "Room ID: " << parent.roomId << " successfully bound to user: " << authenticationRequest.username << std::endl;
    }
    else {
        bindResponse.status = FAILURE_BIND;
        std::cout << "Failed to bind Room ID for user: " << authenticationRequest.username << std::endl;
    }

    comm->sendMessage(Serialize::serializeAuthResponse(bindResponse)); 


    // Receive SDP message 
    receivedMsg = comm->receiveMessage();


    if (receivedMsg[0] == ERROR__TIMEOUT)
    {
        std::cerr << "timouttttttttkbkknnkjn" << std::endl;
    }


    if (authenticationRequest.role == "Parent") {
        std::cout << "Handling SDP offer from parent..." << std::endl;
        handleSDPOffer(hashedSecret, receivedMsg);
    }
    else {
        std::cout << "Handling SDP answer from child..." << std::endl;
        handleSDPAnswer(hashedSecret, receivedMsg);
    }

    std::cout << "Authentication and SDP handling complete for user: " << authenticationRequest.username << std::endl;

    // now peers are connected
    //...handle peers
}



/*
void Server::handleClient(const SOCKET& clientSocket)
{
    


    // Wait for SDP Offer or Response
    std::vector <char> sdpMessage(1024);
    if (authentication.role == "child") {
        sdpMessage = comm->receiveMessage();

        // Save the SDP offer in the database
        db.saveSDPOffer(authentication.username, sdpMessage);

        // Debug: Print the SDP offer
        std::string message(sdpMessage.begin(), sdpMessage.end());
        std::cout << "Received and saved SDP offer: " << message << std::endl;

        // Retrieve the SDP offer from the database
        std::vector<char> sdpAnswer = db.getSDPResponse(authentication.username);

        bool validResponseFound = false;

        while (!validResponseFound) {
            // Retrieve the SDP response from the database
            sdpAnswer = db.getSDPResponse(authentication.username);

            // Check if the response is valid (i.e., not empty)
            if (!sdpAnswer.empty()) {
                validResponseFound = true;
                // Print or process the valid response
                std::cout << "Received valid SDP response for user: " << authentication.username << std::endl;
            }
            else {
                // If no valid response is found, wait for a moment before trying again
                std::cout << "Waiting for valid SDP response..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the wait time as necessary
            }
        }

        if (!sdpAnswer.empty()) {
            // Send the SDP offer to the parent
            comm->sendMessage(sdpAnswer);
            std::cout << "Sent SDP offer to child." << std::endl;

        }
    }
    

            /*std::string otherPeerRole = db.getMatchingPeer(authentication.username, authentication.password, authentication.role);

            // Check if other peer exists and forward the SDP message
            auto otherPeer = clients.find({ authentication.username, otherPeerRole });
            while (sdpMessage.size() > 0 && otherPeer == clients.end()) {
                std::cout << "Waiting for other peer to connect..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                otherPeerRole = db.getMatchingPeer(authentication.username, authentication.password, authentication.role);
                otherPeer = clients.find({ authentication.username, otherPeerRole });
                std::cout << "Found other peer: " << otherPeerRole << std::endl;
            }

            if (otherPeer != clients.end()) {
                otherPeer->second->sendMessage(sdpMessage);
            }
        }
    else
    {
        // Retrieve the SDP offer from the database
        std::vector<char> sdpOffer = db.getSDPOffer(authentication.username);

        if (!sdpOffer.empty()) {
            // Send the SDP offer to the parent
            comm->sendMessage(sdpOffer);
            std::cout << "Sent SDP offer to parent." << std::endl;

        }
        else {
            std::cerr << "No SDP offer found to send to parent." << std::endl;
        }

        sdpMessage = comm->receiveMessage();
        db.saveSDPResponse(authentication.username, sdpMessage);

        // Debug: Print the SDP offer
        std::string message(sdpMessage.begin(), sdpMessage.end());
        std::cout << "Received and saved SDP ANSWER: " << message << std::endl;
    }
      
}
*/