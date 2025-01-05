// LiveImageDisplay.h
#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <mutex>

class LiveImageDisplay {
public:
    void showOneFrame(const std::wstring& initialImagePath);
    void updateImage(const std::wstring& imagePath);
    ~LiveImageDisplay();

private:
    std::mutex imageMutex;

    void showImage(const std::wstring& imagePath, HDC hdc);

    HWND hwnd;
    HDC hdc;
    ULONG_PTR gdiplusToken;
};
