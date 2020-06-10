#pragma once

#include <vector>
#include <functional>
#include <optional>

#include "GameData.h"
#include "CommandListener.h"

enum class ServerState
{
	Lobby,
	GameStart,
	ResetRound,
	PresidentNaturalAdvance,
	PresidentChancellorSelection,
	Election,
	PresidentCardSelect,
	ChancellorCardSelect,
	ChancellorVeto,
	PresidentVeto,
	ElectionChaos,
	PlayCard,
	PresidentInvestigate,
	PresidentNominate,
	PresidentPeek,
	PresidentKill,
	Win,
};

class Server : public CommandListener
{
	ServerState state = ServerState::Lobby;

	std::vector<Policy> discard;
	std::vector<Policy> cards;
	int liberalCards = 0;
	int fascistCards = 0;
	int electionTracker = 0;
	Policy policySideWin; //The policy side that wins the game

	char presidentChair;
	char chancellorChair;

	char lastPresidentChair;
	char lastChancellorChair;

	std::vector<Player> players;

public:
	void addPlayer(Player);
	void loop();

	std::optional<Policy> checkWin();
};
