#include <iostream>
#include <string>
#include <vector>
#include <stdexcept> // For handling exceptions
#include <winsock2.h> // For networking functions (e.g., recv, send)
#include "Communicator.h"
#include "DTLS.h"

int main(){
//int main(int argc, char* argv[]) {
    /*// Check if the port argument is passed
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    // Get the port number from the command-line argument
    int port = 0;
    try {
        port = std::stoi(argv[1]); // Convert string to int
        if (port <= 0 || port > 65535) {
            throw std::out_of_range("Port number must be between 1 and 65535.");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Invalid port number: " << e.what() << std::endl;
        return 1;
    }*/

    int port = 99999;

    Communicator comm;
    comm.connectToOtherPeer(port);

    DTLS dtls;

    dtls.runChildDTLS(comm);
    //dtls.printKeyMaterial();


    std::cout << "\n\n\n\n";

    comm.runChildMain(dtls);

    std::cout << "Debug: Closing client socket." << std::endl;




    std::cin.ignore();
    std::cin.ignore();
    std::cin.get(); std::cin.get();
    std::cin.get(); std::cin.get();
}