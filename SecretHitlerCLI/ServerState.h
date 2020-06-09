#pragma once

#include <vector>
#include <functional>
#include <optional>

#include "GameData.h"
#include "CommandListener.h"

enum class SState
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

class ServerState : public CommandListener
{
	SState state = SState::Lobby;

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

	std::vector<Client> players;

public:
	void addPlayer(Client);
	void loop();

	std::optional<Policy> checkWin();
};
