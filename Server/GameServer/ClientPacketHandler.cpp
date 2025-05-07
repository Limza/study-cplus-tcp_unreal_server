#include "pch.h"
#include "ClientPacketHandler.h"

#include "GameSession.h"
#include "Player.h"
#include "Room.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, const int32 len)
{
	auto header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO: 잘못된 패킷 처리에 대한 Log
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO: Validation Check

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	static Atomic<uint64> idGenerator = 1;

	{
		const auto player = loginPkt.add_players();
		player->set_name(reinterpret_cast < const char*>(u8"DB에서 긁어온 이름 1"));
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		const auto playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->playerType = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.emplace_back(playerRef);
	}

	{
		const auto player = loginPkt.add_players();
		player->set_name(reinterpret_cast <const char*>(u8"DB에서 긁어온 이름 2"));
		player->set_playertype(Protocol::PLAYER_TYPE_MAGE);

		const auto playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->playerType = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.emplace_back(playerRef);
	}

	const auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pkt.playerindex();
	// TODO: index 유효성 체크

	gameSession->_currentPlayer = gameSession->_players[index];
	gameSession->_room = GRoom;

	GRoom->DoAsync(&Room::Enter, gameSession->_currentPlayer);

	Protocol::S_ENTER_GAME enterGamePkt;
	enterGamePkt.set_success(true);
	const auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	gameSession->_currentPlayer->ownerSession->Send(sendBuffer);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	std::cout << pkt.msg() << '\n';

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	const auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}

