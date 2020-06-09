#include <iostream>
#include <string>
#include <random>
#include <map>

#include <WinSock2.h>

#include "ServerState.h"
#include "Packets.h"

using std::to_string;

template<typename T>
void shuffle(std::vector<T> v);

template<typename T>
void send(SOCKET sock, Client client, T packet);

template<typename T>
void bounce(SOCKET sock, T packet);

sockaddr_in Client::addr()
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = endpoint.address;
	addr.sin_port = endpoint.port;
	return addr;
}

void ServerState::loop()
{
	using namespace Packets;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		throw std::runtime_error("Socket failed: " + to_string(WSAGetLastError()));
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(6660);

	if (bind(sock, (SOCKADDR*) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		throw std::runtime_error("Bind failed: " + to_string(WSAGetLastError()));
	}

	u_long enabled = 1;
	ioctlsocket(sock, FIONBIO, &enabled);

	std::vector<Policy> selectionCards;
	std::map<char, bool> electionVotes;
	int electionChaos;
	while (true)
	{
		sockaddr_in from;
		int fromlen = sizeof(from);

		bool hasMessage = true;
		char buffer[4096];
		int r = recvfrom(sock, buffer, 4096, 0, (sockaddr*) &from, &fromlen);

		Endpoint ep = { from.sin_addr.s_addr, from.sin_port };
		auto player =
			std::find_if(
				players.begin(), players.end(),
				[&ep](Client c) { return c.endpoint.combo == ep.combo; });

		auto message = (ClientNonePacket*) buffer;

		if (r == SOCKET_ERROR)
		{
			hasMessage = false;
			int e = WSAGetLastError();
			if (e != WSAEWOULDBLOCK)
			{
				std::cout << "Receive failed: " << e << std::endl;
			}
		}

		static std::vector<Client> alivePlayers;
		switch (state)
		{
		case SState::Lobby:
			if (hasMessage)
			{
				switch (message->kind)
				{
				case ClientPacketKind::Join:
					{
						auto data = (ClientJoinContent*) &message->content;
						Client c;
						c.endpoint = ep;
						c.pData.len = data->nameLen;
						for (int i = 0; i < data->nameLen; i++)
							c.pData.name[i] = data->name[i];
						players.push_back(c);
					}
					break;
				case ClientPacketKind::Ready:
					{
						auto data = (ClientReadyContent*) &message->content;
						if (player != players.end())
						{
							player->ready = data->state;
						}
					}
					break;
				}
			}

			{
				bool allReady = true;
				for (auto p : players)
					if (!p.ready)
						allReady = false;

				if (players.size() >= 5 && allReady)
				{
					state = SState::GameStart;
				}
			}
			break;
		case SState::GameStart:
			//Creates and shuffles policy deck
			cards.clear();
			cards.insert(cards.end(), 6, Policy::Liberal); //Append 6 Lib cards
			cards.insert(cards.end(), 11, Policy::Fascist); //Append 11 Fac cards
			shuffle(cards);

			//Assign players association
			shuffle(players);
			players[0].data.isHitler = true;
			for (int i = 0; i <= (players.size() - 5) / 2 + 1; i++)
			{
				players[i].data.isFascist = true;
			}
			shuffle(players);

			for (int i = 0; i < players.size(); i++)
			{
				players[i].pData.chair = i;
				send(sock, players[i], ServerPlayerDataPacket({ players[i].pData }));
			}

			ServerPlayerListContent content;
			content.nPlayers = players.size();
			for (int i = 0; i < players.size(); i++)
			{
				content.players[i] = players[i].pData;
			}
			bounce(sock, ServerPlayerListPacket(content));

			alivePlayers = players;

			electionChaos = 0;

			state = SState::PresidentChancellorSelection; //Skip the president selection because the president has automatically been selected at the start of the game
			break;
		case SState::PresidentNaturalAdvance:
			lastPresidentChair = presidentChair;
			lastChancellorChair = chancellorChair;

			//If the proposed player is dead continue trying the next player
			while(players[presidentChair = ++presidentChair % players.size()].alive != true);

			bounce(sock, ServerNewPresidentPacket({ presidentChair }));

			state = SState::PresidentChancellorSelection;
			break;
		case SState::PresidentChancellorSelection:
			if (hasMessage && message->kind == ClientPacketKind::ChancellorPick)
			{
				auto data = (ClientChancellorPickContent*) &message->content;
				if (player->pData.chair == presidentChair)
				if (data->chancellor != player->pData.chair)
				if (data->chancellor != lastChancellorChair)
				{
					if (alivePlayers.size() > 5 || data->chancellor != lastPresidentChair)
					{
						chancellorChair = data->chancellor;
						state = SState::Election;

						bounce(sock, ServerElectionRequestPacket({ chancellorChair }));
						electionVotes.clear();
					}
				}
			}
			break;
		case SState::Election:
			if (hasMessage && message->kind == ClientPacketKind::ElectionVote)
			{
				auto data = (ClientElectionVoteContent*) &message->content;
				electionVotes[player->pData.chair] = data->state;
			}

			{
				int yes = 0, no = 0;
				for (auto [chair, vote] : electionVotes)
				{
					if (vote) yes++;
					else no++;
				}

				if (electionVotes.size() == alivePlayers.size())
				{
					if (yes > no)
					{
						state = SState::PresidentCardSelect;
						ServerCardListContent cardList;
						cardList.nCards = 3;

						for (int i = 0; i < 3; i++)
						{
							selectionCards.push_back(cards.front());
							cards.erase(cards.begin() + i);

							cardList.cards[i] = selectionCards[i];
						}

						send(sock, players[presidentChair], cardList);
					}
					else
					{
						if (++electionChaos == 4)
						{
							selectionCards.push_back(cards.front());
							cards.erase(cards.begin());
							state = SState::ElectionChaos;
						}
						else
						{
							bounce(sock, ServerElectionChaosPacket());
						}
					}
				}
			}
			break;
		case SState::PresidentCardSelect:
			if (hasMessage && message->kind == ClientPacketKind::CardPick)
			{
				auto data = (ClientCardPickContent*) &message->content;
				selectionCards.erase(selectionCards.begin() + data->card);
				ServerCardListContent cardList;
				cardList.nCards = 2;
				for (int i = 0; i < 2; i++)
				{
					cardList.cards[i] = selectionCards[i];
				}
				send(sock, players[chancellorChair], ServerCardListPacket(cardList));
				state = SState::ChancellorCardSelect;
			}
			break;
		case SState::ChancellorCardSelect:
			if (hasMessage)
			{
				switch (message->kind)
				{
				case ClientPacketKind::CardPick:
					auto data = (ClientCardPickContent*) &message->content;
					selectionCards.erase(selectionCards.begin() + data->card);
					state = SState::PlayCard;
					break;
				case ClientPacketKind::Veto:
					if (fascistCards == 5)
					{
						send(sock, player[presidentChair], ServerVetoRequestPacket());
						state = SState::PresidentVeto;
					}
					break;
				}
			}
			break;
		case SState::PresidentVeto:
			if (hasMessage && message->kind == ClientPacketKind::Veto)
			{
				auto data = (ClientVetoContent*) &message->content;
				if (data->state)
				{
					electionChaos++;
					if (electionChaos == 4)
					{
						cards.insert(cards.begin(), selectionCards.back());
						selectionCards.pop_back();
						state = SState::ElectionChaos;
					}
					else
					{
						state = SState::PresidentNaturalAdvance;
					}
				}
				else
				{
					send(sock, players[chancellorChair], ServerForcePlayRequestPacket());
					state = SState::ChancellorCardSelect;
				}
			}
			break;
		case SState::ElectionChaos:
			bounce(sock, ServerElectionChaosPacket());
			state = SState::PlayCard;
			break;
		case SState::PlayCard:
			break;
		}
	}
}

void ServerState::addPlayer(Client ce)
{
	if (players.size() == 10)
	{
		std::cout << "Too many players attempted to join!" << std::endl;
		return;
	}
	players.push_back(ce);
}

std::optional<Policy> ServerState::checkWin()
{
	if (fascistCards >= 3 && players[chancellorChair].data.isHitler || fascistCards == 6)
		return std::make_optional(Policy::Fascist);
	if (liberalCards == 5)
		return std::make_optional(Policy::Liberal);
	return std::nullopt;
}

template <typename T>
void shuffle(std::vector<T> v)
{
	static std::mt19937 dev(time(0));
	std::uniform_int_distribution rand(0, (int) v.size() - 1);

	for (unsigned r = 0; r < 3; r++)
	for (unsigned i = 0; i < v.size(); i++)
	{
		unsigned j = rand(dev);
		T temp = v[i];
		v[i] = v[j];
		v[j] = temp;
	}
}

template<typename T>
void send(SOCKET sock, Client client, T packet)
{
	sockaddr_in addr = client.addr();
	int addrLen = sizeof(addr);
	sendto(sock, (char*) &packet, sizeof(T), 0, (sockaddr*) &addr, addrLen);
}

template<typename T>
void bounce(SOCKET sock, T packet)
{
	send(sock, (char*) &packet, sizeof(T), 0);
}
