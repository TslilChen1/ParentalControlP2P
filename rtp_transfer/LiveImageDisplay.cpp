// LiveImageDisplay.cpp
#include "LiveImageDisplay.h"
#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>  // הוספת ספריית האלגוריתמים עבור std::min
#include "LockImage.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

template <typename T>
T minValue(T a, T b) {
    return (a < b) ? a : b;
}


void LiveImageDisplay::showOneFrame(const std::wstring& initialImagePath) {
    // GDI+ initialization
    GdiplusStartupInput gdiStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiStartupInput, NULL);

    // Get console window
    hwnd = GetConsoleWindow();
    hdc = GetDC(hwnd);

    updateImage(L"output.png");
}

void LiveImageDisplay::updateImage(const std::wstring& imagePath) {
    showImage(imagePath, hdc);
}

void LiveImageDisplay::showImage(const std::wstring& imagePath, HDC hdc) {
    Bitmap bitmap(imagePath.c_str());
    if (bitmap.GetLastStatus() != Ok) {
        std::cerr << "Failed to load the image!" << std::endl;
        return;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    UINT width = bitmap.GetWidth();
    UINT height = bitmap.GetHeight();

    float scaleX = (float)screenWidth / width;
    float scaleY = (float)screenHeight / height;
    float scale = minValue(scaleX, scaleY);

    int newWidth = (int)(width * scale);
    int newHeight = (int)(height * scale);

    Graphics graphics(hdc);
    graphics.Clear(Color(255, 255, 255, 255));
    graphics.DrawImage(&bitmap, 0, 0, newWidth, newHeight);
}

LiveImageDisplay::~LiveImageDisplay() {
    ReleaseDC(hwnd, hdc);
    GdiplusShutdown(gdiplusToken);
}