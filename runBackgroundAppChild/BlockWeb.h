#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h>

class BlockWeb
{
public:
	static void addToHostsFile(const std::string& url);
	static void removeFromHostsFile(const std::string& url);
	static void flushDNS();	
	static void clearBrowserCache();
	static void blockSite(const std::string& baseUrl);
	static void unblockSite(const std::string& baseUrl);
	static void closeBrowsers();
	static void openBrowsers();
	static bool isValidUrl(const std::string& url);
};

	