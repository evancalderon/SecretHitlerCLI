#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <random>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Client.h"
#include "Packets.h"
#include "Util.h"

using std::to_string;

class ClientSocket
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

		u_long enabled = 1;
		ioctlsocket(sock, FIONBIO, &enabled);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(6660);
	}

	template<typename T>
	unsigned send(T data)
	{
		return ::sendto(sock, (char*) &data, sizeof(data), 0, (sockaddr*) &addr, addrLen);
	}

	unsigned recv(char* buffer, unsigned bufferLen)
	{
		return recvfrom(sock, (char*) buffer, bufferLen, 0, (sockaddr*) &addr, &addrLen);
	}
};

void Client::loop()
{
	using namespace Packets;

	ClientSocket socket;

	while (true)
	{
		static bool joined;

		addCommand("join", [&](StringList args) {
			if (args.size() < 2)
			{
				std::cout << "Missing name." << std::endl;
				return;
			}
			std::string name = args[1];
			ClientJoinContent join;
			join.len = min(name.size(), 30);
			for (int i = 0; i < join.len; i++)
			{
				join.name[i] = name[i];
			}
			socket.send(ClientJoinPacket(join));
			joined = true;
		});
		execCommand();

		if (joined)
		{
			break;
		}
	}

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

		PlayerData pdata;

		switch (state)
		{
		case ClientState::Lobby:
			addCommand("ready", [&](StringList args)
			{
				socket.send(ClientReadyPacket({ true }));
			});
			addCommand("unready", [&](StringList args)
			{
				socket.send(ClientReadyPacket({ false }));
			});
			execCommand();

			if (hasMessage && message->kind == ServerPacketKind::GameStart)
			{
				state = ClientState::Game;
			}

			break;
		case ClientState::Game:
			if (hasMessage)
			{
				switch (message->kind)
				{
				case ServerPacketKind::PlayerData:
					{
						auto data = (ServerPlayerDataContent*) &message->content;
						clientData = data->data;
						std::string role;
						if (clientData->isHitler)
						{
							role = "Hitler";
						}
						else if (clientData->isFascist)
						{
							role = "Fascist";
						}
						else
						{
							role = "Liberal";
						}
						std::cout <<
							"You are a " << role <<
							" and you are at seat #" <<
							int(clientData->chair) << "." << std::endl;
					}
					break;
				case ServerPacketKind::PlayerList:
					{
						auto data = (ServerPlayerListContent*) &message->content;
						std::cout << "Players: " << std::endl;
						for (int i = 0; i < data->nPlayers; i++)
						{
							std::cout <<
								std::string(
									data->players[i].name,
									data->players[i].len) <<
								std::endl;
						}
					}
				}
			}
			break;
		}
	}
}
