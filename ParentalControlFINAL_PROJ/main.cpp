#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include "Communicator.h"

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <role>" << std::endl;
            std::cerr << "Example: " << argv[0] << " Parent" << std::endl;
            return 1;
        }

        Communicator communicator;

        std::string role = argv[1];

        if (role == "Parent") {
            communicator.runAsClient("127.0.0.1");
        }
        else if (role == "Child") {
            communicator.runAsServer();
        }
        else {
            std::cerr << "Invalid role: " << role << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Main error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
