#pragma once

#include <optional>

#include "GameData.h"
#include "CommandListener.h"

enum class CState
{
};

class ClientState : public CommandListener
{
	CState state = CState::Join;
	std::optional<ClientData> data;
public:
	void loop();
};
