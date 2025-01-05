#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    
    port = "12355";

    // Make sure to use the full path to the executable
    std::string installCommand = "sc create ChildParentalControllService binPath= \"C:\\Users\\Magshimim\\Documents\\Magshimim\\or-akiva-504-parentalcontrolp2p\\peers\\ConsoleApplication1.exe\"";

    // Debug: print the command to see it
    std::cout << "Install Command: " << installCommand << std::endl;

    const char* startCommand = R"(sc start ChildParentalControllService)";

    // Install the service
    if (std::system(installCommand.c_str()) != 0) {
        std::cerr << "Failed to install the service.\n";
        return 1;
    }

    std::cout << "Service installed successfully.\n";

    // Start the service
    if (std::system(startCommand) != 0) {
        std::cerr << "Failed to start the service.\n";
        return 1;
    }

    std::cout << "Service started successfully.\n";

    std::cin.ignore();
    std::cin.ignore();
    std::cin.get(); std::cin.get();

    return 0;
}
