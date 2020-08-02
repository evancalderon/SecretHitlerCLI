#include <iostream>
#include <string>
#include <random>
#include <map>

#include <WinSock2.h>

#include "Server.h"
#include "Packets.h"

using std::to_string;

template<typename T>
void shuffle(std::vector<T> v);

template<typename T>
void send(SOCKET sock, Player client, T packet);

template<typename T>
void bounce(SOCKET sock, T packet);

sockaddr_in Player::addr()
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = endpoint.address;
	addr.sin_port = endpoint.port;
	return addr;
}

void Server::loop()
{
	using namespace Packets;

	auto sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

	std::vector<Policy> selection_cards;
	std::map<char, bool> election_votes;
	int election_chaos;
	while (true)
	{
		sockaddr_in from {};
		int fromlen = sizeof(from);

		auto has_message = true;
		char buffer[4096];
		auto r = recvfrom(sock, buffer, 4096, 0, (sockaddr*) &from, &fromlen);

		Endpoint ep = { from.sin_addr.s_addr, from.sin_port };
		auto player =
			std::find_if(
				players.begin(), players.end(),
				[&ep](Player c) { return c.endpoint.combo == ep.combo; });

		auto* message = (ClientNonePacket*) buffer;

		if (r == SOCKET_ERROR)
		{
			has_message = false;
			auto e = WSAGetLastError();
			if (e != WSAEWOULDBLOCK)
			{
				std::cout << "Receive failed: " << e << std::endl;
			}
		}

		if (has_message && message->kind == ClientPacketKind::Claim)
		{
			auto pc = player->p_data.chair;
			if (pc == presidentChair || pc == lastPresidentChair ||
				pc == chancellorChair || pc == lastChancellorChair)
			{
				auto* data = (ClientClaimContent*) &message->content;
				ServerClaimContent claim {};
				claim.chair = player->p_data.chair;
				claim.nCards = data->nCards;
				for (auto i = 0; i < 3; i++)
				{
					claim.cards[i] = data->cards[i];
				}
				bounce(sock, ServerClaimContent(claim));
			}
		}

		static std::vector<Player> alivePlayers;
		switch (state)
		{
		case ServerState::Lobby:
			if (has_message)
			{
				switch (message->kind)
				{
				case ClientPacketKind::Join:
					{
						auto* data = (ClientJoinContent*) &message->content;
						Player c;
						c.endpoint = ep;
						c.p_data.len = data->len;
						*c.p_data.name = *data->name;
						players.push_back(c);
						std::cout << "Player joined: " << c.p_data.str() << std::endl;
					}
					break;
				case ClientPacketKind::Ready:
					{
						auto* data = (ClientReadyContent*) &message->content;
						if (player != players.end())
						{
							player->ready = data->state;
							auto ready = 0;
							for (auto p : players)
							{
								if (p.ready)
									ready++;
							}
							std::cout << ready << " out of " <<
								players.size() << " ready." << std::endl;
						}
					}
					break;
				default:
					break;
				}
			}

			{
				auto allReady = true;
				for (auto p : players)
					if (!p.ready)
						allReady = false;

				if (players.size() >= 5 && allReady)
				{
					bounce(sock, ServerGameStartPacket());
					state = ServerState::GameStart;
				}
			}
			break;
		case ServerState::GameStart:
			//Creates and shuffles policy deck
			cards.clear();
			cards.insert(cards.end(), 6, Policy::Liberal); //Append 6 Lib cards
			cards.insert(cards.end(), 11, Policy::Fascist); //Append 11 Fac cards
			shuffle(cards);

			//Assign players association
			shuffle(players);
			players[0].data.is_hitler = true;
			for (auto i = 0; i <= (players.size() - 5) / 2 + 1; i++)
			{
				players[i].data.is_fascist = true;
			}
			shuffle(players);

			for (auto i = 0; i < players.size(); i++)
			{
				ServerPlayerDataContent content {};
				content.data.chair = i;
				content.data.is_fascist = players[i].data.is_fascist;
				content.data.is_hitler = players[i].data.is_hitler;
				players[i].p_data.chair = i;
				send(sock, players[i], ServerPlayerDataPacket(content));
			}

			ServerPlayerListContent content {};
			content.nPlayers = players.size();
			for (auto i = 0; i < players.size(); i++)
			{
				content.players[i] = players[i].p_data;
			}
			bounce(sock, ServerPlayerListPacket(content));

			switch (alivePlayers.size())
			{
			case 6:
			case 8:
			case 10:
				fascistCards++;
			default:
				break;
			}

			alivePlayers = players;

			election_chaos = 0;

			//Skip the president selection because the president has
			// automatically been selected at the start of the game
			bounce(sock, ServerNewPresidentPacket({ presidentChair }));
			state = ServerState::PresidentChancellorSelection;
			break;
		case ServerState::PresidentNaturalAdvance:
			lastPresidentChair = presidentChair;
			lastChancellorChair = chancellorChair;

			//If the proposed player is dead continue trying the next player
			while(players[presidentChair = ++presidentChair % players.size()].alive != true)
			{
			}

			bounce(sock, ServerNewPresidentPacket({ presidentChair }));

			state = ServerState::PresidentChancellorSelection;
			break;
		case ServerState::PresidentChancellorSelection:
			if (has_message && message->kind == ClientPacketKind::ChancellorPick)
			{
				auto* data = (ClientChancellorPickContent*) &message->content;
				if (player->p_data.chair == presidentChair)
				if (data->chancellorChair != player->p_data.chair)
				if (data->chancellorChair != lastChancellorChair)
				{
					if (alivePlayers.size() > 5 || data->chancellorChair != lastPresidentChair)
					{
						chancellorChair = data->chancellorChair;
						state = ServerState::Election;

						bounce(sock, ServerElectionRequestPacket({ chancellorChair }));
						election_votes.clear();
					}
				}
			}
			break;
		case ServerState::Election:
			if (has_message && message->kind == ClientPacketKind::ElectionVote)
			{
				auto* data = (ClientElectionVoteContent*) &message->content;
				election_votes[player->p_data.chair] = data->state;
			}

			{
				auto yes = 0, no = 0;
				for (auto [chair, vote] : election_votes)
				{
					if (vote) yes++;
					else no++;
				}

				if (election_votes.size() == alivePlayers.size())
				{
					bounce(sock, ServerElectionResultPacket({ yes > no }));
					if (yes > no)
					{
						state = ServerState::PresidentCardSelect;
						ServerCardListContent card_list {};
						card_list.nCards = 3;

						for (auto i = 0; i < 3; i++)
						{
							selection_cards.push_back(cards.front());
							cards.erase(cards.begin() + i);

							card_list.cards[i] = selection_cards[i];
						}

						send(sock, players[presidentChair], card_list);
					}
					else
					{
						if (++election_chaos == 4)
						{
							selection_cards.push_back(cards.front());
							cards.erase(cards.begin());
							state = ServerState::ElectionChaos;
						}
						else
						{
							bounce(sock, ServerElectionChaosPacket());
						}
					}
				}
			}
			break;
		case ServerState::PresidentCardSelect:
			if (has_message && message->kind == ClientPacketKind::CardPick)
			{
				auto* data = (ClientCardPickContent*) &message->content;
				selection_cards.erase(selection_cards.begin() + data->card);
				ServerCardListContent card_list {};
				card_list.nCards = 2;
				for (auto i = 0; i < 2; i++)
				{
					card_list.cards[i] = selection_cards[i];
				}
				send(sock, players[chancellorChair], ServerCardListPacket(card_list));
				state = ServerState::ChancellorCardSelect;
			}
			break;
		case ServerState::ChancellorCardSelect:
			if (has_message)
			{
				switch (message->kind)
				{
				case ClientPacketKind::CardPick:
					{
						auto* data = (ClientCardPickContent*) &message->content;
						selection_cards.erase(selection_cards.begin() + data->card);
						state = ServerState::PlayCard;
					}
					break;
				case ClientPacketKind::Veto:
					if (fascistCards == 5)
					{
						send(sock, player[presidentChair], ServerVetoRequestPacket());
						state = ServerState::PresidentVeto;
					}
					break;
				default:
					break;
				}
			}
			break;
		case ServerState::PresidentVeto:
			if (has_message && message->kind == ClientPacketKind::Veto)
			{
				auto* data = (ClientVetoContent*) &message->content;
				if (data->state)
				{
					election_chaos++;
					if (election_chaos == 4)
					{
						cards.insert(cards.begin(), selection_cards.back());
						selection_cards.pop_back();
						state = ServerState::ElectionChaos;
					}
					else
					{
						bounce(sock, ServerElectionChaosPacket());
						state = ServerState::PresidentNaturalAdvance;
					}
				}
				else
				{
					send(sock, players[chancellorChair], ServerForcePlayRequestPacket());
					state = ServerState::ChancellorCardSelect;
				}
			}
			break;
		case ServerState::ElectionChaos:
			bounce(sock, ServerElectionChaosPacket());
			state = ServerState::PlayCard;
			break;
		case ServerState::PlayCard:
			{
				//Sends which card was played to clients
				bounce(sock, ServerCardPlayedContent({ selection_cards[0] }));

				if (selection_cards[0] == Policy::Fascist)
					fascistCards++;
				else
					liberalCards++;

				auto win = checkWin();
				if (win)
				{
					//Assign the value of the winning party to variable for the Win state
					policySideWin = win.value();
					state = ServerState::Win;
					// TODO: Create Win state
				}
				else
				{
					switch (fascistCards)
					{
					case 1:
						state = ServerState::PresidentPeek;
						break;
					case 2:
						send(sock, players[presidentChair], ServerInvestigationRequestPacket());
						state = ServerState::PresidentInvestigate;
						break;
					case 3:
						send(sock, players[presidentChair], ServerNominateRequestPacket());
						state = ServerState::PresidentNominate;
						break;
					case 4:
					case 5:
						send(sock, players[presidentChair], ServerKillRequestPacket());
						state = ServerState::PresidentKill;
						break;
					default:
						break;
					}
				}

				selection_cards.clear();
			}
			break;
		case ServerState::PresidentPeek:
			{
				ServerPeekedCardsPacket msg;
				for (auto i = 0; i < 3; i++)
				{
					msg.content.cards[i] = cards[i];
				}
				send(sock, players[presidentChair], msg);
			}
			state = ServerState::PresidentNaturalAdvance;
			break;
		case ServerState::PresidentInvestigate:
			if (has_message && message->kind == ClientPacketKind::Investigate)
			{
				auto* data = (ClientInvestigateContent*) &message->content;
				send(sock, players[presidentChair],
					ServerInvestigationReportPacket({
						data->chair, players[data->chair].data.is_fascist }));
			}
			break;
		case ServerState::PresidentNominate:
			if (has_message && message->kind == ClientPacketKind::Nominate)
			{
				auto* data = (ClientNominateContent*) &message->content;
				presidentChair = data->chair;
				bounce(sock, ServerNewPresidentPacket({ presidentChair }));
				state = ServerState::PresidentCardSelect;
			}
			break;
		case ServerState::PresidentKill:
			if (has_message && message->kind == ClientPacketKind::Kill)
			{
				auto* data = (ClientKillContent*) &message->content;
				players[data->chair].alive = false;
				bounce(sock, ServerInformDeathPacket({ data->chair }));
			}
			break;
		case ServerState::Win:
			{
				bounce(sock, ServerAnnounceWinContent({ policySideWin }));

				if (policySideWin == Policy::Fascist)
					std::cout << "Fascists";
				else
					std::cout << "Liberals";
				std::cout << " Win!" << std::endl;

				return;
			}
		default:
			break;
		}
	}
}

void Server::addPlayer(Player ce)
{
	if (players.size() == 10)
	{
		std::cout << "Too many players attempted to join!" << std::endl;
		return;
	}
	players.push_back(ce);
}

std::optional<Policy> Server::checkWin()
{
	if (fascistCards >= 3 && players[chancellorChair].data.is_hitler || fascistCards == 6)
		return std::make_optional(Policy::Fascist);
	if (liberalCards == 5)
		return std::make_optional(Policy::Liberal);
	return std::nullopt;
}

template <typename T>
void shuffle(std::vector<T> v)
{
	static std::mt19937 dev(rand());
	const std::uniform_int_distribution rand(0, (int) v.size() - 1);

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
void send(SOCKET sock, Player client, T packet)
{
	auto addr = client.addr();
	const int addrLen = sizeof addr;
	sendto(sock, (char*) &packet, sizeof(T), 0, (sockaddr*) &addr, addrLen);
}

template<typename T>
void bounce(SOCKET sock, T packet)
{
	send(sock, (char*) &packet, sizeof(T), 0);
}
