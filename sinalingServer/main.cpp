#include "Server.h"
#include <exception>
#include <iostream>

int main()
{
	try
	{
		WSADATA wsa_data = { };
		if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
			throw std::exception("WSAStartup Failed");
	}
	catch (const std::exception& e)
	{
		std::cout << "WSAStartup Failed" << std::endl;
		exit(1);
	}
	std::shared_ptr <SQLiteWrapper> db = std::make_shared<SQLiteWrapper>();

	Server server(db);
	server.start();
}
