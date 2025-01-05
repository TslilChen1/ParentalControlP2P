#pragma once
#include <string>
class LockImage
{
public:
	static void unlockFile(const std::wstring& filePath);
    static bool lockFile(const std::wstring& filePath);
};
