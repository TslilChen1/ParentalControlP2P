#include "RemoteControlingRequestHandler.h"
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

std::string ServerMain() {
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
        return "";
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
    std::string str(buffer);;
    return str;
}


std::vector<char> RemoteControlingRequestHandler::convertToVector(const TurnOfComputerResponse& response) {
    std::vector<char> result;
    result.push_back(static_cast<uint8_t>(response.status));
    return result;
}

IRequestHandler::RequestResult RemoteControlingRequestHandler::handleRequest(const RequestInfo& request) {
    std::cout << "got into handleRequest func" << std::endl;
    switch (request.id)
    {
    case TURN_OFF_COMPUTER_REQUEST:
        return turnOffComputer();
    case KEY_BOARD_READING_REQUEST:
        KeyBoardReadingRequest keyBoardReadingRequest = Deserializer::deserializeKeyBoardReadingRequest(request.buffer);
        return readKeyboard(keyBoardReadingRequest.timeInSecondToRead);
    case BLOCK_WEB_REQUEST:
        BlockingWebRequest blockingWebRequest;
        try
        {
            blockingWebRequest = Deserializer::deserializeBlockingWebRequest(request.buffer);

        }
        catch (const std::exception& e)
        {
            std::cerr << "Error during deserialization: " << e.what() << std::endl;
            return IRequestHandler::RequestResult{ {}, nullptr };
        }
        return blockWeb(blockingWebRequest.webName);
    }

    // if command not found
    return IRequestHandler::RequestResult{ {}, nullptr };
    
}

IRequestHandler::RequestResult RemoteControlingRequestHandler::turnOffComputer() {
    /*
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Open the process token to adjust privileges
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        std::cerr << "OpenProcessToken failed with error: " << GetLastError() << std::endl;
        {
            return IRequestHandler::RequestResult{ {}, nullptr };;
        }
    }

    // Get the LUID for the shutdown privilege
    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
        std::cerr << "LookupPrivilegeValue failed with error: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return IRequestHandler::RequestResult{ {}, nullptr };;
    }

    tkp.PrivilegeCount = 1;  // One privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  // Enable the shutdown privilege

    // Adjust the token privileges
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        std::cerr << "AdjustTokenPrivileges failed with error: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return IRequestHandler::RequestResult{ {}, nullptr };
    }

    // Check if the adjustment was successful
    if (GetLastError() != ERROR_SUCCESS)
    {
        std::cerr << "Failed to enable shutdown privilege. Error: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        {
            {
                return IRequestHandler::RequestResult{ {}, nullptr };
            }
        }
    }

    // Shut down the computer
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER))
    //if (!ExitWindowsEx(EWX_SHUTDOWN, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER)) {
    {
        std::cerr << "ExitWindowsEx failed with error: " << GetLastError() << std::endl;
    }

    // Close the token handle
    CloseHandle(hToken);
    */
    std::vector<char> response = { 'O', 'K' };  // sending back "OK"
    return IRequestHandler::RequestResult{ response, nullptr };
}

IRequestHandler::RequestResult RemoteControlingRequestHandler::readKeyboard(int timeInSeconds)
{
    std::string capturedKeys = ServerMain();

    // output the captured keys after the loop finishes
    std::vector<char> response(capturedKeys.begin(), capturedKeys.end());
    return IRequestHandler::RequestResult{ response, nullptr };
}

IRequestHandler::RequestResult RemoteControlingRequestHandler::blockWeb(std::string webName)
{
    BlockWeb::closeBrowsers();
    BlockWeb::blockSite(webName);
    BlockWeb::flushDNS();
    BlockWeb::clearBrowserCache();
    BlockWeb::openBrowsers();
    std::cout << webName << " and its subdomains have been blocked." << std::endl;

    std::string res = "success";

    // output the captured keys after the loop finishes
    std::vector<char> response(res.begin(), res.end());
    return IRequestHandler::RequestResult{ response, nullptr };
}



bool RemoteControlingRequestHandler::isRequestRelevant(const RequestInfo& info) const {
    return info.id == TURN_OFF_COMPUTER_REQUEST || info.id == KEY_BOARD_READING_REQUEST;
}
