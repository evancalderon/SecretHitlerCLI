#pragma once

#include "GameData.h"

namespace Packets
{
	struct NoContent
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
		Investigate,
		Nominate,
		Kill,
		Claim,
	};

	template<ClientPacketKind K, typename T>
	struct ClientPacket
	{
		ClientPacketKind kind = K;
		T content;

		ClientPacket(T content = {}): content(content)
		{
		}
	};

	struct ClientJoinContent
	{
		char len;
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
		char chancellorChair;
	};

	struct ClientCardPickContent
	{
		char card;
	};

	struct ClientVetoContent
	{
		bool state;
	};

	struct ClientInvestigateContent
	{
		char chair;
	};

	struct ClientNominateContent
	{
		char chair;
	};

	struct ClientKillContent
	{
		char chair;
	};

	struct ClientClaimContent
	{
		char nCards;
		Policy cards[3];
	};

	typedef ClientPacket<ClientPacketKind::None, NoContent>
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
	typedef ClientPacket<ClientPacketKind::Investigate, ClientInvestigateContent>
		ClientInvestigatePacket;
	typedef ClientPacket<ClientPacketKind::Nominate, ClientNominateContent>
		ClientNominatePacket;
	typedef ClientPacket<ClientPacketKind::Claim, ClientClaimContent>
		ClientClaimPacket;
	typedef ClientPacket<ClientPacketKind::Kill, ClientKillContent>
		ClientKillPacket;

	#pragma endregion

	#pragma region ServerPacket

	enum class ServerPacketKind
	{
		None,
		GameStart,
		PlayerData,
		NewPresident,
		PlayerList,
		ElectionRequest,
		ElectionChaos,
		CardList,
		VetoRequest,
		ForcePlayRequest,
		CardPlayed,
		AnnounceWin,
		PeekedCards,
		InvestigationRequest,
		InvestigationReport,
		NominateRequest,
		KillRequest,
		Claim,
		InformDeath,
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

	struct ServerCardListContent
	{
		char nCards;
		Policy cards[3];
	};

	struct ServerCardPlayedContent
	{
		Policy cardPlayed;
	};

	struct ServerAnnounceWinContent
	{
		Policy winningSide;
	};

	struct ServerPeekedCardsContent
	{
		Policy cards[3];
	};

	struct ServerInvestigationReportContent
	{
		char chair;
		bool isFascist;
	};

	struct ServerClaimContent
	{
		char chair;
		char nCards;
		Policy cards[3];
	};

	struct ServerInformDeathContent
	{
		char chair;
	};

	typedef ServerPacket<ServerPacketKind::None, NoContent>
		ServerNonePacket;
	typedef ServerPacket<ServerPacketKind::GameStart, NoContent>
		ServerGameStartPacket;
	typedef ServerPacket<ServerPacketKind::PlayerData, ServerPlayerDataContent>
		ServerPlayerDataPacket;
	typedef ServerPacket<ServerPacketKind::NewPresident, ServerNewPresidentContent>
		ServerNewPresidentPacket;
	typedef ServerPacket<ServerPacketKind::PlayerList, ServerPlayerListContent>
		ServerPlayerListPacket;
	typedef ServerPacket<ServerPacketKind::ElectionRequest, ServerElectionRequestContent>
		ServerElectionRequestPacket;
	typedef ServerPacket<ServerPacketKind::ElectionChaos, NoContent>
		ServerElectionChaosPacket;
	typedef ServerPacket<ServerPacketKind::CardList, ServerCardListContent>
		ServerCardListPacket;
	typedef ServerPacket<ServerPacketKind::VetoRequest, NoContent>
		ServerVetoRequestPacket;
	typedef ServerPacket<ServerPacketKind::ForcePlayRequest, NoContent>
		ServerForcePlayRequestPacket;
	typedef ServerPacket<ServerPacketKind::CardPlayed, ServerCardPlayedContent>
		ServerCardPlayedPacket;
	typedef ServerPacket<ServerPacketKind::AnnounceWin, ServerAnnounceWinContent>
		ServerAnnounceWinPacket;
	typedef ServerPacket<ServerPacketKind::PeekedCards, ServerPeekedCardsContent>
		ServerPeekedCardsPacket;
	typedef ServerPacket<ServerPacketKind::InvestigationRequest, NoContent>
		ServerInvestigationRequestPacket;
	typedef ServerPacket<ServerPacketKind::InvestigationReport, ServerInvestigationReportContent>
		ServerInvestigationReportPacket;
	typedef ServerPacket<ServerPacketKind::NominateRequest, NoContent>
		ServerNominateRequestPacket;
	typedef ServerPacket<ServerPacketKind::KillRequest, NoContent>
		ServerKillRequestPacket;
	typedef ServerPacket<ServerPacketKind::Claim, ServerClaimContent>
		ServerClaimPacket;
	typedef ServerPacket<ServerPacketKind::InformDeath, ServerInformDeathContent>
		ServerInformDeathPacket;
	
	#pragma endregion

}
