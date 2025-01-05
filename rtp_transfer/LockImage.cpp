#include "LockImage.h"
#include <windows.h>
#include <iostream>


// I DIDNT USE IT BUT MAY BE GOOD FOR OTHER TIME
void LockImage::unlockFile(const std::wstring& filePath) {
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file!" << std::endl;
        return;
    }

    OVERLAPPED overlapped = { 0 };
    UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &overlapped);
    std::cout << "File unlocked successfully!" << std::endl;

    CloseHandle(hFile);
}

bool LockImage::lockFile(const std::wstring& filePath)
{

    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file!" << std::endl;
        return false;
    }

    OVERLAPPED overlapped = { 0 };
    BOOL locked = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped);

    if (!locked) {
        std::cerr << "Failed to lock the file!" << std::endl;
        CloseHandle(hFile);
        return false;
    }

    std::cout << "File locked successfully!" << std::endl;
    return true;
}
