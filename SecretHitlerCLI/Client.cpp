#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <algorithm>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Client.h"
#include "Packets.h"
#include "Util.h"

using std::to_string;

class ClientSocket
{
	SOCKET sock{};
	sockaddr_in addr{};
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

	template <typename T>
	unsigned send(T data)
	{
		return sendto(sock, static_cast<char*>(&data), sizeof(data), 0, (sockaddr*)&addr, addrLen);
	}

	unsigned recv(char* buffer, unsigned bufferLen)
	{
		return recvfrom(sock, static_cast<char*>(buffer), bufferLen, 0, (sockaddr*)&addr, &addrLen);
	}
};

void Client::loop()
{
	using namespace Packets;

	ClientSocket socket;

	while (true)
	{
		static bool joined;

		addCommand("join", [&](StringList args)
		{
			if (args.size() < 2)
			{
				std::cout << "Missing name." << std::endl;
				return;
			}
			auto name = args[1];
			ClientJoinContent join;
			join.len = min(name.size(), 30);
			for (auto i = 0; i < join.len; i++)
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

	ServerCardListContent current_cards;
	auto force_play = false;
	while (true)
	{
		auto has_message = true;
		char buffer[4096];
		const int r = socket.recv(buffer, 4096);

		auto* message = (ServerNonePacket*)buffer;

		if (r == SOCKET_ERROR)
		{
			has_message = false;
			const auto e = WSAGetLastError();
			if (e != WSAEWOULDBLOCK)
			{
				std::cout << "Receive failed: " << e << std::endl;
			}
		}

		PlayerData pdata;

		switch (state)
		{
		case ClientState::Lobby:
			addCommand("ready", [&](StringList /*args*/)
			{
				socket.send(ClientReadyPacket({true}));
			});
			addCommand("unready", [&](const StringList& /*args*/)
			{
				socket.send(ClientReadyPacket({false}));
			});

			if (has_message && message->kind == ServerPacketKind::GameStart)
			{
				state = ClientState::Game;
			}

			break;
		case ClientState::Game:
			if (has_message)
			{
				switch (message->kind)
				{
				case ServerPacketKind::PlayerData:
					{
						auto* data = (ServerPlayerDataContent*)&message->content;
						clientData = data->data;
						std::string role;
						if (clientData->is_hitler)
						{
							role = "Hitler";
						}
						else if (clientData->is_fascist)
						{
							role = "a Fascist";
						}
						else
						{
							role = "a Liberal";
						}
						std::cout <<
							"You are " << role <<
							" and take seat #" <<
							static_cast<int>(clientData->chair) <<
							"." << std::endl;
					}
					break;
				case ServerPacketKind::PlayerList:
					{
						auto* data = (ServerPlayerListContent*)&message->content;
						std::cout << "Players: " << std::endl;
						players.clear();
						players.reserve(data->nPlayers);
						players.resize(data->nPlayers);
						for (auto i = 0; i < data->nPlayers; i++)
						{
							auto& player = data->players[i];
							players[player.chair] = player;
							std::cout << data->players[i].str() <<
								std::endl;
						}

						std::cout << players.size() << "p game" << std::endl;
						switch (players.size())
						{
						case 6:
						case 8:
						case 10:
							std::cout << "A fascist card has been added to even the odds" << std::endl;
							break;
						default:
							break;
						}
					}
					break;
				case ServerPacketKind::NewPresident:
					{
						auto* data = (ServerNewPresidentContent*)&message->content;
						presidentChair = data->newPresidentChair;
						auto& pres = players[presidentChair];
						if (presidentChair == clientData->chair)
						{
							std::cout << "You have";
						}
						else
						{
							std::cout << pres.str() << " has";
						}
						std::cout << " been made president." << std::endl;

						if (presidentChair == clientData->chair)
						{
							state = ClientState::ChancellorPicking;
						}
					}
					break;
				case ServerPacketKind::ElectionRequest:
					{
						auto* data = (ServerElectionRequestContent*)&message->content;
						chancellorChair = data->newChancellorChair;
						std::cout <<
							"Election for " << players[chancellorChair].str() <<
							", chosen by " << players[presidentChair].str() << ", has begun."
							<< std::endl;
						state = ClientState::Election;
					}
					break;
				case ServerPacketKind::CardList:
					{
						auto* data = (ServerCardListContent*)&message->content;
						current_cards = *data;

						std::cout << "Cards to choose from: ";
						for (auto i = 0; i < current_cards.nCards; i++)
						{
							const auto card = current_cards.cards[i];
							switch (card)
							{
							case Policy::Fascist:
								std::cout << "R";
							case Policy::Liberal:
								std::cout << "B";
							}
						}
						std::cout << std::endl;

						state = ClientState::PickCard;
					}
					break;
				case ServerPacketKind::VetoRequest:
					if (clientData->chair == presidentChair)
					{
						std::cout << "The chancellor has voted to veto the current card set." << std::endl;
						std::cout << "Do you wish to accept? (accept, decline)" << std::endl;
						state = ClientState::VetoRequest;
					}
					break;
				default:
					break;
				case ServerPacketKind::ForcePlayRequest:
					if (clientData->chair == chancellorChair)
					{
						force_play = true;
						std::cout << "The president has denied the veto. You are required to choose a card." << std::endl;
						state = ClientState::PickCard;
					}
				case ServerPacketKind::CardPlayed:
					{
						auto* data = (ServerCardPlayedContent*) &message->content;
						switch (data->cardPlayed)
						{
						case Policy::Fascist:
							std::cout << "A fascist card was played." << std::endl;
							break;
						case Policy::Liberal:
							std::cout << "A liberal card was played." << std::endl;
							break;
						}
					}
				case ServerPacketKind::AnnounceWin:
					{
						auto* data = (ServerAnnounceWinContent*) &message->content;
						switch (data->winningSide)
						{
						case Policy::Fascist:
							std::cout << "The fascists have won." << std::endl;
							break;
						case Policy::Liberal:
							std::cout << "The liberals have won." << std::endl;
							break;
						}
					}
				}
			}
			break;
		case ClientState::ChancellorPicking:
			addCommand("pick", [&](StringList args)
			{
				if (args[1] == "player")
				{
					std::cout << "Not implemented." << std::endl;
				}
				else if (args[1] == "seat")
				{
					char chair;
					try
					{
						chair = std::stoi(args[2]);
					}
					catch (std::invalid_argument e)
					{
						std::cout << "Invalid number." << std::endl;
						return;
					}
					catch (std::exception e)
					{
						std::cout << "An unknown error occurred." << std::endl;
						return;
					}
					if (chair < 1 || chair > players.size())
					{
						std::cout << "Invalid seat." << std::endl;
						return;
					}
					socket.send(ClientChancellorPickPacket({static_cast<char>(chair)}));
					state = ClientState::Game;
				}
			});
			break;
		case ClientState::Election:
			addCommand("vote", [&](StringList args)
			{
				auto vote_str = args[1];
				std::transform(vote_str.begin(), vote_str.end(), vote_str.begin(), std::tolower);
				bool vote;
				if (vote_str == "yes" || vote_str == "jah")
					vote = true;
				else if (vote_str == "no" || vote_str == "nein")
					vote = false;
				else
				{
					std::cout << R"(Possible options: "yes", "no", "jah", "nein")";
					return;
				}

				socket.send(ClientElectionVotePacket({vote}));
			});

			if (has_message && message->kind == ServerPacketKind::ElectionResult)
			{
				auto* data = (ServerElectionResultContent*)&message->content;
				std::cout <<
					("Election " + std::string(data->success ? "succeeded. " : "failed. ")) <<
					players[chancellorChair].str() << " is now chancellor." << std::endl;
				state = ClientState::Game;
			}
			break;
		case ClientState::PickCard:
			addCommand("remove", [&](StringList args)
			{
				if (args.size() < 2)
				{
					std::cout << "Must pick a card number." << std::endl;
				}
				char card;
				try
				{
					card = std::stoi(args[1]);
				}
				catch (std::invalid_argument e)
				{
					std::cout << "Invalid number." << std::endl;
					return;
				}
				catch (std::exception e)
				{
					std::cout << "An unknown exception occurred." << std::endl;
					return;
				}
				if (card < 1 || card > current_cards.nCards)
				{
					std::cout << "Invalid card." << std::endl;
					return;
				}
				socket.send(ClientCardPickPacket({card}));
				state = ClientState::Game;
			});
			if (clientData->chair == chancellorChair && !force_play)
			{
				addCommand("veto", [&](StringList args)
				{
					socket.send(ClientVetoPacket({true}));
					state = ClientState::Game;
				});
			}
			break;
		case ClientState::VetoRequest:
			addCommand("accept", [&](StringList args)
			{
				socket.send(ClientVetoPacket({true}));
				state = ClientState::Game;
			});
			addCommand("decline", [&](StringList args)
			{
				socket.send(ClientVetoPacket({false}));
				state = ClientState::Game;
			});
			break;
		}
		execCommand();
	}
}
