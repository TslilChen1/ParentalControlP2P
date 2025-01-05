#include "ScreenCapture.h"
#include <string>
#include <iostream>
//#include <C:/opencv/build/include/opencv2/opencv.hpp>
using namespace Gdiplus;
#include <thread>
#include <chrono>

/*
int ScreenCapture::showImage()
{
	// Read the image file as 
  // imread("default.jpg"); 
	Mat image = imread("screen.png", IMREAD_COLOR);

	// Error Handling 
	if (image.empty()) {
		cout << "Image File "
			<< "Not Found" << endl;

		// wait for any key press 
		cin.get();
		return -1;
	}

	// Show Image inside a window with 
	// the name provided 
	imshow("Window Name", image);

	// Wait for any keystroke 
	waitKey(0);
	return 0;
}
*/

int ScreenCapture::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	using namespace Gdiplus;
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return 0;
}

void ScreenCapture::gdiscreen() {
	IStream* istream;
	HRESULT res = CreateStreamOnHGlobal(NULL, true, &istream);
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	{
		HDC scrdc, memdc;
		HBITMAP membit;
		scrdc = ::GetDC(0);
		int Height = GetSystemMetrics(SM_CYSCREEN);
		int Width = GetSystemMetrics(SM_CXSCREEN);
		memdc = CreateCompatibleDC(scrdc);
		membit = CreateCompatibleBitmap(scrdc, Width, Height);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);
		BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

		Gdiplus::Bitmap bitmap(membit, NULL);
		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);
		bitmap.Save(L"screen.png", &clsid, NULL); // To save the jpeg to a file
		bitmap.Save(istream, &clsid, NULL);

		//delete &clsid;
		DeleteObject(memdc);
		DeleteObject(membit);
		::ReleaseDC(0, scrdc);
	}
	GdiplusShutdown(gdiplusToken);
}
