#pragma once

#include <optional>

#include "GameData.h"
#include "CommandListener.h"

enum class ClientState
{
	Lobby,
	Game,
};

class Client : public CommandListener
{
	ClientState state = ClientState::Lobby;
	std::optional<ClientData> data;
public:
	void loop();
};
