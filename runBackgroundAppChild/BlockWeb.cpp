#include "BlockWeb.h"
#include <vector>

void BlockWeb::addToHostsFile(const std::string& url)
{
    std::ofstream hostsFile("C:\\Windows\\System32\\drivers\\etc\\hosts", std::ios::app);
    if (hostsFile.is_open()) {
        hostsFile << "127.0.0.1 " << url << std::endl;
        std::cout << "Added " << url << " to hosts file." << std::endl;
        hostsFile.close();
    }
    else {
        std::cerr << "Failed to open hosts file. Run as administrator." << std::endl;
    }
}

void BlockWeb::removeFromHostsFile(const std::string& url)
{
    std::string tempFile = "C:\\Windows\\System32\\drivers\\etc\\hosts_temp";
    std::ifstream hostsFile("C:\\Windows\\System32\\drivers\\etc\\hosts");
    std::ofstream tempHostsFile(tempFile);
    std::string line;

    if (hostsFile.is_open() && tempHostsFile.is_open()) {
        while (getline(hostsFile, line)) {
            if (line.find(url) == std::string::npos) {
                tempHostsFile << line << std::endl;
            }
        }
        hostsFile.close();
        tempHostsFile.close();
        std::remove("C:\\Windows\\System32\\drivers\\etc\\hosts");
        std::rename(tempFile.c_str(), "C:\\Windows\\System32\\drivers\\etc\\hosts");
        std::cout << "Removed " << url << " from hosts file." << std::endl;
    }
    else {
        std::cerr << "Failed to open hosts file or temp file. Run as administrator." << std::endl;
    }
}

void BlockWeb::flushDNS()
{
    std::string command = "ipconfig /flushdns";
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "DNS cache flushed successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to flush DNS cache." << std::endl;
    }
}

void BlockWeb::clearBrowserCache()
{
    std::cout << "Clearing browser cache for Google Chrome, Mozilla Firefox, and Microsoft Edge..." << std::endl;
    std::string command = "RunDll32.exe InetCpl.cpl,ClearMyTracksByProcess 8";
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "Browser cache cleared successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to clear browser cache." << std::endl;
    }
    std::cout << "You may need to restart your browser for the changes to take effect." << std::endl;
}

void BlockWeb::blockSite(const std::string& baseUrl)
{
    addToHostsFile(baseUrl);
    addToHostsFile("www." + baseUrl);
    addToHostsFile("m." + baseUrl);
}

void BlockWeb::unblockSite(const std::string& baseUrl)
{
    removeFromHostsFile(baseUrl);
    removeFromHostsFile("www." + baseUrl);
    removeFromHostsFile("m." + baseUrl);
}

void BlockWeb::closeBrowsers()
{
    std::cout << "Closing all open browsers..." << std::endl;
    system("taskkill /F /IM chrome.exe >nul 2>&1");
    system("taskkill /F /IM msedge.exe >nul 2>&1");
}

void BlockWeb::openBrowsers()
{
    std::cout << "Reopening browsers..." << std::endl;
    system("start chrome");
    system("start msedge");
}



