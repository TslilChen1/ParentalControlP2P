#include <windows.h>
#include <iostream>
#include <processthreadsapi.h>
#define PIPE_NAME L"\\\\.\\pipe\\MyPipe"  // Name of the pipe

void LaunchClient() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the client process
    if (!CreateProcess(
        L"PipeClient.exe",  // Path to your client executable
        NULL,              // Command line arguments
        NULL,              // Process handle not inheritable
        NULL,              // Thread handle not inheritable
        FALSE,             // Set handle inheritance to FALSE
        0,                 // No creation flags
        NULL,              // Use parent's environment block
        NULL,              // Use parent's starting directory 
        &si,              // Pointer to STARTUPINFO structure
        &pi               // Pointer to PROCESS_INFORMATION structure
    )) {
        std::cerr << "CreateProcess failed with error: " << GetLastError() << std::endl;
        return;
    }

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void ServerMain() {
    HANDLE hPipe;
    DWORD dwRead;
    char buffer[1024];
    // Create the named pipe
    hPipe = CreateNamedPipe(PIPE_NAME,
        PIPE_ACCESS_INBOUND,  // The pipe is for reading
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,  // Output buffer size
        1024,  // Input buffer size
        0,
        NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed with error: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Launching client process..." << std::endl;
    LaunchClient();  // Launch the client executable

    std::cout << "Waiting for client connection..." << std::endl;
    // Wait for the client to connect
    if (ConnectNamedPipe(hPipe, NULL) != FALSE) {
        std::cout << "Client connected!" << std::endl;
       // Sleep(10);
        // Read the data sent from the client
        if (ReadFile(hPipe, buffer, sizeof(buffer), &dwRead, NULL)) {
            buffer[dwRead] = '\0';  // Null-terminate the string
            std::cout << "Server received: " << buffer << std::endl;
        }
        else {
            std::cerr << "Failed to read data from pipe. Error: " << GetLastError() << std::endl;
        }
        // Close the pipe
        CloseHandle(hPipe);
    }
    else {
        std::cerr << "Failed to connect to pipe. Error: " << GetLastError() << std::endl;
    }
}

int main() {
    // Run the server code
    ServerMain();
    return 0;
}