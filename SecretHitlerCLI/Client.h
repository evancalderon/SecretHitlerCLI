#pragma once

#include <optional>

#include "GameData.h"
#include "CommandListener.h"

enum class ClientState
{
	Lobby,
	Game,
	ChancellorPicking,
	Election,
	PickCard,
	VetoRequest,
};

class Client : public CommandListener
{
	ClientState state = ClientState::Lobby;
	std::optional<ClientData> clientData;
	std::vector<PlayerData> players;
	
	char presidentChair = 0;
	char chancellorChair = 0;
public:
	void loop();
};
