#pragma once

#include "GameData.h"

namespace Packets
{
	struct NoneContent
	{
	};

	#pragma region ClientPacket

	enum class ClientPacketKind
	{
		None,
		Join,
		Ready,
		ElectionVote,
		ChancellorPick,
		CardPick,
		Veto,
	};

	template<ClientPacketKind K, typename T>
	struct ClientPacket
	{
		ClientPacketKind kind = K;
		T content;

		ClientPacket(T content): content(content)
		{
		}
	};

	struct ClientJoinContent
	{
		unsigned nameLen;
		char name[32];
	};

	struct ClientReadyContent
	{
		bool state;
	};

	struct ClientElectionVoteContent
	{
		bool state;
	};

	struct ClientChancellorPickContent
	{
		char chancellor;
	};

	struct ClientCardPickContent
	{
		char card;
	};

	struct ClientVetoContent
	{
		bool state;
	};

	typedef ClientPacket<ClientPacketKind::None, NoneContent>
		ClientNonePacket;
	typedef ClientPacket<ClientPacketKind::Join, ClientJoinContent>
		ClientJoinPacket;
	typedef ClientPacket<ClientPacketKind::Ready, ClientReadyContent>
		ClientReadyPacket;
	typedef ClientPacket<ClientPacketKind::ElectionVote, ClientElectionVoteContent>
		ClientElectionVotePacket;
	typedef ClientPacket<ClientPacketKind::ChancellorPick, ClientChancellorPickContent>
		ClientChancellorPickPacket;
	typedef ClientPacket<ClientPacketKind::CardPick, ClientCardPickContent>
		ClientCardPickPacket;
	typedef ClientPacket<ClientPacketKind::Veto, ClientVetoContent>
		ClientVetoPacket;

	#pragma endregion

	#pragma region ServerPacket

	enum class ServerPacketKind
	{
		None,
		PlayerData,
		NewPresident,
		PlayerList,
		ElectionRequest,
		ElectionChaos,
		CardList,
		VetoRequest,
		ForcePlayRequest,
		CardPlayed,
		SendWin,
	};

	template<ServerPacketKind K, class T>
	struct ServerPacket
	{
		ServerPacketKind kind = K;
		T content;

		ServerPacket(T content = {}): content(content)
		{
		}
	};

	struct ServerPlayerDataContent
	{
		PlayerData data;
	};

	struct ServerNewPresidentContent
	{
		char newPresidentChair;
	};

	struct ServerPlayerListContent
	{
		char nPlayers;
		PlayerData players[10];
	};

	struct ServerElectionRequestContent
	{
		char newChancellorChair;
	};

	struct ServerElectionChaosContent
	{
	};

	struct ServerCardListContent
	{
		int nCards;
		Policy cards[3];
	};

	struct ServerVetoRequestContent
	{
	};

	struct ServerForcePlayRequestContent
	{
	};

	struct ServerCardPlayedContent
	{
		Policy cardPlayed;
	};

	struct ServerSendWinContent
	{
		Policy winningSide;
	};

	typedef ServerPacket<ServerPacketKind::None, NoneContent>
		ServerNonePacket;
	typedef ServerPacket<ServerPacketKind::PlayerData, ServerPlayerDataContent>
		ServerPlayerDataPacket;
	typedef ServerPacket<ServerPacketKind::NewPresident, ServerNewPresidentContent>
		ServerNewPresidentPacket;
	typedef ServerPacket<ServerPacketKind::PlayerList, ServerPlayerListContent>
		ServerPlayerListPacket;
	typedef ServerPacket<ServerPacketKind::ElectionRequest, ServerElectionRequestContent>
		ServerElectionRequestPacket;
	typedef ServerPacket<ServerPacketKind::ElectionChaos, ServerElectionChaosContent>
		ServerElectionChaosPacket;
	typedef ServerPacket<ServerPacketKind::CardList, ServerCardListContent>
		ServerCardListPacket;
	typedef ServerPacket<ServerPacketKind::VetoRequest, ServerVetoRequestContent>
		ServerVetoRequestPacket;
	typedef ServerPacket<ServerPacketKind::ForcePlayRequest, ServerForcePlayRequestContent>
		ServerForcePlayRequestPacket;
	typedef ServerPacket<ServerPacketKind::CardPlayed, ServerCardPlayedContent>
		ServerCardPlayedPacket;
	typedef ServerPacket<ServerPacketKind::SendWin, ServerSendWinContent>
		ServerSendWinContent;
	
	#pragma endregion

}
