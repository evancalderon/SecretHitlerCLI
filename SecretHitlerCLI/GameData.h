#pragma once

#include <WinSock2.h>

enum class Policy
{
	Fascist,
	Liberal,
};

struct PlayerData
{
	char len;
	char name[30];
	char chair;
};

struct ClientData
{
	char chair;
	bool isFascist;
	bool isHitler;
};

union Endpoint
{
	struct
	{
		unsigned int address;
		unsigned short port;
	};
	unsigned long combo;
};

struct Player
{
	Endpoint endpoint;
	ClientData data;
	PlayerData pData;
	bool alive = true;
	bool ready = false;

	sockaddr_in addr();
};
