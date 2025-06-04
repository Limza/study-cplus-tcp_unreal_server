#include "pch.h"
#include "Room.h"

#include <ranges>

#include "Player.h"
#include "Protocol.pb.h"
#include "ServerPacketHandler.h"

RoomRef GRoom = MakeShared<Room>();

Room::Room()
{
}

Room::~Room()
{
}

bool Room::HandleEnterPlayerLocked(const PlayerRef& player)
{
	WRITE_LOCK;

	const bool success = EnterPlayer(player);

	player->playerInfo->set_x(Utils::GetRandom(0.f, 500.f));
	player->playerInfo->set_y(Utils::GetRandom(0.f, 500.f));
	player->playerInfo->set_z(100.f);
	player->playerInfo->set_yaw(Utils::GetRandom(0.f, 500.f));

	// 입장 사실을 Enter Player 에게 알린다
	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);

		const auto playerInfo = new Protocol::PlayerInfo();
		playerInfo->CopyFrom(*player->playerInfo);
		enterGamePkt.set_allocated_player(playerInfo);

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
		if (const auto session = player->session.lock())
			session->Send(sendBuffer);
	}

	// 입장 사실을 Other Player 에게 알린다
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
		playerInfo->CopyFrom(*player->playerInfo);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, player->playerInfo->object_id());
	}

	// 기존 입장한 플레이어 목록을 신입 플레이어한테 전송한다
	{
		Protocol::S_SPAWN spawnPkt;

		for (const auto& enterPlayer : _players | std::views::values)
		{
			if (enterPlayer->playerInfo->object_id() == player->playerInfo->object_id())
				continue;
			Protocol::PlayerInfo* playerInfo = spawnPkt.add_players();
			playerInfo->CopyFrom(*enterPlayer->playerInfo);
		}

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
		if (const auto session = player->session.lock())
			session->Send(sendBuffer);
	}

	return success;
}

bool Room::HandleLeavePlayerLocked(const PlayerRef& player)
{
	if (player == nullptr)
		return false;

	WRITE_LOCK;

	PlayerRef playerRef = player;

	const uint64 objectId = playerRef->playerInfo->object_id();
	bool success = LeavePlayer(objectId);

	// 퇴장 사실을 퇴장하는 플레이어에게 알린다
	{
		const Protocol::S_LEAVE_GAME leaveGamePkt;

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(leaveGamePkt);
		if (const auto session = playerRef->session.lock())
			session->Send(sendBuffer);
	}

	// 퇴장 사실을 룸에 알린다
	{
		Protocol::S_DESPAWN despawnPkt;
		despawnPkt.add_object_ids(objectId);

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(despawnPkt);

		Broadcast(sendBuffer);

		// LeavePlayer() 함수에서 Broadcast 대상에서 빠지기 때문에, 여기서 다시 send
		if (auto session = playerRef->session.lock())
			session->Send(sendBuffer);
	}

	return success;
}

void Room::HandleMoveLocked(const Protocol::C_MOVE& pkt)
{
	WRITE_LOCK;

	const uint64 objectId = pkt.info().object_id();
	if (_players.contains(objectId) == false)
		return;

	// TODO : pkt 정보가 유효한지 확인

	PlayerRef& player = _players[objectId];
	player->playerInfo->CopyFrom(pkt.info());

	{
		Protocol::S_MOVE movePkt;
		{
			Protocol::PlayerInfo* info = movePkt.mutable_info();
			info->CopyFrom(pkt.info());
		}

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer);
	}
}

bool Room::EnterPlayer(const PlayerRef& player)
{
	if (_players.contains(player->playerInfo->object_id()))
		return false;

	PlayerRef playerRef = player; // Add ref
	_players.insert(std::make_pair(player->playerInfo->object_id(), playerRef));
	playerRef->room.store(shared_from_this());

	return true;
}

bool Room::LeavePlayer(const uint64 objectId)
{
	if (_players.contains(objectId) == false)
		return false;

	const PlayerRef player = _players[objectId];
	player->room.store(std::weak_ptr<Room>());

	_players.erase(objectId);

	return true;
}

void Room::Broadcast(const SendBufferRef& sendBuffer, const uint64 exceptId)
{
	SendBufferRef sendBufferRef = sendBuffer; // ADD Ref

	for (const auto& player : _players | std::views::values)
	{
		if (player->playerInfo->object_id() == exceptId)
			continue;

		if (const GameSessionRef session = player->session.lock())
			session->Send(sendBufferRef);
	}
}
