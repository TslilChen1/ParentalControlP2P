/*#include <windows.h>
#include <tchar.h>
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS


SERVICE_STATUS g_ServiceStatus = { 0 };             // Current status of the service
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;      // Handle to the service status
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE; // Handle to signal service stop

void ConvertLPTSTRToCharArray(LPTSTR lpsz, char* charArray, size_t size) {
    WideCharToMultiByte(CP_ACP, 0, lpsz, -1, charArray, size, NULL, NULL);
}
LPWSTR ConvertToLPWSTR(const char* charString) {
    // Determine the length of the new wide string
    int size_needed = MultiByteToWideChar(CP_ACP, 0, charString, -1, NULL, 0);

    // Allocate memory for the wide string
    LPWSTR wideString = new wchar_t[size_needed];

    // Perform the conversion
    MultiByteToWideChar(CP_ACP, 0, charString, -1, wideString, size_needed);

    return wideString;
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
        {
            // Stop the service
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

            // Signal the service to stop
            SetEvent(g_ServiceStopEvent);
        }
        break;

    default:
        break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    // Register the service control handler
    g_StatusHandle = RegisterServiceCtrlHandler(TEXT("ChildParentalControllService"), ServiceCtrlHandler);
    if (g_StatusHandle == NULL)
    {
        return;
    }

    // Initialize the service status
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    // Report service is starting
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    
    
    // Create the stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Report service is running
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Service main loop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {

        // Make sure an argument (port) is passed
        if (argc < 2) {
            std::cerr << "No port argument passed!" << std::endl;
            return;
        }

        char port[256];
        ConvertLPTSTRToCharArray(argv[1], port, sizeof(port));

        // Path to the executable
        const char* exePath = "C:/Users/Magshimim/Documents/Magshimim/or-akiva-504-parentalcontrolp2p/peers/runBackgroundAppChild.exe";

        // Combine the path and port into a single command line
        std::string commandLine = std::string(exePath) + " " + port;
        
        // Convert the command line to a writable format for CreateProcess
        char cmdLine[MAX_PATH];
        strncpy_s(cmdLine, MAX_PATH, commandLine.c_str(), _TRUNCATE);

        // Set up the process startup info and process information structures
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        // Initialize memory for STARTUPINFO
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);  // Set the size of the structure

        // Initialize memory for PROCESS_INFORMATION
        ZeroMemory(&pi, sizeof(pi));

        // Create the process
        if (CreateProcess(
                NULL,               // No module name (use command line)
            ConvertToLPWSTR(cmdLine),            // Command line
                NULL,               // Process handle not inheritable
                NULL,               // Thread handle not inheritable
                FALSE,              // Set handle inheritance to FALSE
                0,                  // No creation flags
                NULL,               // Use parent's environment block
                NULL,               // Use parent's starting directory 
                &si,                // Pointer to STARTUPINFO structure
                &pi)                // Pointer to PROCESS_INFORMATION structure
        ) {
        // Successfully created the process; wait for it to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        std::cout << "Executable ran successfully with argument 'Child'." << std::endl;
        }
        else {
            // Failed to create the process
            std::cerr << "Failed to run executable. Error code: " << GetLastError() << std::endl;
        }
        Sleep(1000);  // Simulate some work
    }

    // Clean up and stop service
    CloseHandle(g_ServiceStopEvent);
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

int _tmain(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {const_cast<LPTSTR>(TEXT("ChildParentalControllService")), ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        return GetLastError();
    }

   
    return 0;
}
*/


#include <windows.h>
#include <tchar.h>
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS


SERVICE_STATUS g_ServiceStatus = { 0 };             // Current status of the service
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;      // Handle to the service status
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE; // Handle to signal service stop

LPWSTR ConvertToLPWSTR(const char* charString) {
    // Determine the length of the new wide string
    int size_needed = MultiByteToWideChar(CP_ACP, 0, charString, -1, NULL, 0);

    // Allocate memory for the wide string
    LPWSTR wideString = new wchar_t[size_needed];

    // Perform the conversion
    MultiByteToWideChar(CP_ACP, 0, charString, -1, wideString, size_needed);

    return wideString;
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
        {
            // Stop the service
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

            // Signal the service to stop
            SetEvent(g_ServiceStopEvent);
        }
        break;

    default:
        break;
    }
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    // Register the service control handler
    g_StatusHandle = RegisterServiceCtrlHandler(TEXT("ChildParentalControllService"), ServiceCtrlHandler);
    if (g_StatusHandle == NULL)
    {
        return;
    }

    // Initialize the service status
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    // Report service is starting
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);


    // Create the stop event
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Report service is running
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Service main loop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {

        // Path to the executable
        const char* exePath = "C:/Users/Magshimim/Documents/Magshimim/or-akiva-504-parentalcontrolp2p/peers/runBackgroundAppChild.exe";

        std::string commandLine = std::string(exePath);

        // Convert the command line to a writable format for CreateProcess
        char cmdLine[MAX_PATH];
        strncpy_s(cmdLine, MAX_PATH, commandLine.c_str(), _TRUNCATE);

        // Set up the process startup info and process information structures
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        // Initialize memory for STARTUPINFO
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);  // Set the size of the structure

        // Initialize memory for PROCESS_INFORMATION
        ZeroMemory(&pi, sizeof(pi));

        // Create the process
        if (CreateProcess(
            NULL,               // No module name (use command line)
            ConvertToLPWSTR(cmdLine),            // Command line
            NULL,               // Process handle not inheritable
            NULL,               // Thread handle not inheritable
            FALSE,              // Set handle inheritance to FALSE
            0,                  // No creation flags
            NULL,               // Use parent's environment block
            NULL,               // Use parent's starting directory 
            &si,                // Pointer to STARTUPINFO structure
            &pi)                // Pointer to PROCESS_INFORMATION structure
            ) {
            // Successfully created the process; wait for it to finish
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Close process and thread handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            std::cout << "Executable ran successfully with argument 'Child'." << std::endl;
        }
        else {
            // Failed to create the process
            std::cerr << "Failed to run executable. Error code: " << GetLastError() << std::endl;
        }
        Sleep(1000);  // Simulate some work
    }

    // Clean up and stop service
    CloseHandle(g_ServiceStopEvent);
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

int _tmain(int argc, TCHAR* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {const_cast<LPTSTR>(TEXT("ChildParentalControllService")), ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        return GetLastError();
    }


    return 0;
}
