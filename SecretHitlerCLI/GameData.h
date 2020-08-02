#pragma once

#include <string>

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

	std::string str();
};

struct ClientData
{
	char chair;
	bool is_fascist;
	bool is_hitler;
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
	PlayerData p_data;
	bool alive = true;
	bool ready = false;

	sockaddr_in addr();
};
