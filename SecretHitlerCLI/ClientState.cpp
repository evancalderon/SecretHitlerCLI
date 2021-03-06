#include <iostream>
#include <string>
#include <random>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientState.h"
#include "Packets.h"

using std::to_string;

class ClientSocket //Abstraction to
{
	SOCKET sock {};
	sockaddr_in addr {};
	int addrLen = sizeof(addr);
public:
	ClientSocket()
	{
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == INVALID_SOCKET)
		{
			throw std::runtime_error("Socket failed: " + to_string(WSAGetLastError()));
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = 0x7f000001;
		addr.sin_port = htons(6660);
	}

	template<typename T>
	unsigned send(T data)
	{
		return sendto(sock, &data, sizeof(T), 0, (sockaddr*) &addr, addrLen);
	}

	unsigned recv(char* buffer, unsigned bufferLen)
	{
		return recvfrom(sock, buffer, bufferLen, 0, (sockaddr*) &addr, &addrLen);
	}
};

void Client::loop()
{
	using namespace Packets;

	ClientSocket socket;

	while (true)
	{
		bool hasMessage = true;
		char buffer[4096];
		int r = socket.recv(buffer, 4096);

		auto message = (ServerNonePacket*) buffer;

		if (r == SOCKET_ERROR)
		{
			hasMessage = false;
			int e = WSAGetLastError();
			if (e != WSAEWOULDBLOCK)
			{
				std::cout << "Receive failed: " << e << std::endl;
			}
		}
	}
}
