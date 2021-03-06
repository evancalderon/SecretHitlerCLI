#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>

#include <WinSock2.h>

#include "Server.h"
#include "Client.h"

using std::to_string;

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		throw std::runtime_error("WSAStartup failed: " + to_string(WSAGetLastError()));
	}

	std::string i;
	std::getline(std::cin, i);

	std::unique_ptr<CommandListener> listener;
	if (i == "host")
	{
		listener = std::make_unique<Server>();
	}
	else
	{
		listener = std::make_unique<Client>();
	}

	std::thread thread([&listener]()
	{
		listener->loop();
	});

	while (true)
	{
		std::string input;
		std::getline(std::cin, input);

		listener->submit(input);
	}
}
