#include "pch.h"
#include "ServerPacketHandler.h"

#include "GameSession.h"
#include "ObjectUtils.h"
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
	Protocol::S_LOGIN loginPkt;

	// TODO : 캐릭터가 3개 있다고 가정
	for (int32 i = 0; i < 3; ++i)
	{
		Protocol::PlayerInfo* player = loginPkt.add_players();
		player->set_x(Utils::GetRandom(0.f, 100.f));
		player->set_y(Utils::GetRandom(0.f, 100.f));
		player->set_z(Utils::GetRandom(0.f, 100.f));
		player->set_yaw(Utils::GetRandom(0.f, 45.f));
	}

	loginPkt.set_success(true);

	SEND_PACKET(loginPkt, session);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	PlayerRef player = ObjectUtils::CreatePlayer(
		std::static_pointer_cast<GameSession>(session));

	GRoom->HandleEnterPlayerLocked(player);

	return true;
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	const auto gameSession = std::static_pointer_cast<GameSession>(session);

	const PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	const RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->HandleLeavePlayerLocked(player);

	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	const auto gameSession = std::static_pointer_cast<GameSession>(session);

	const PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	const RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->HandleMoveLocked(pkt);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	std::cout << pkt.msg() << '\n';

	return true;
}

