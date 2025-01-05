#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>  // הוספת ספריית האלגוריתמים עבור std::min

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

class ScreenCapture
{ 
public:
	//static int showImage();
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static void gdiscreen();
};



