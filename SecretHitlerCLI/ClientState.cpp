#include <iostream>
#include <string>
#include <random>

#include <WinSock2.h>

#include "ClientState.h"

using std::to_string;

void ClientState::loop()
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock == INVALID_SOCKET)
	{
		throw std::runtime_error("Socket failed: " + to_string(WSAGetLastError()));
	}

	while (true)
	{
	}
}
