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

bool Room::EnterRoom(ObjectRef object, bool randPos)
{
	const bool success = AddObject(object);

	// 랜덤 위치
	if (randPos)
	{
		object->posInfo->set_x(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_y(Utils::GetRandom(0.f, 500.f));
		object->posInfo->set_z(100.f);
		object->posInfo->set_yaw(Utils::GetRandom(0.f, 500.f));
	}

	// 입장 사실을 Enter Player 에게 알린다
	if (const auto player = std::dynamic_pointer_cast<Player>(object))
	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(success);

		const auto playerInfo = new Protocol::ObjectInfo();
		playerInfo->CopyFrom(*player->objectInfo);
		enterGamePkt.set_allocated_player(playerInfo);

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
		if (const auto session = player->session.lock())
			session->Send(sendBuffer);
	}

	// 입장 사실을 Other Player 에게 알린다
	{
		Protocol::S_SPAWN spawnPkt;

		Protocol::ObjectInfo* objectInfo = spawnPkt.add_players();
		objectInfo->CopyFrom(*object->objectInfo);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(sendBuffer, object->objectInfo->object_id());
	}

	// 기존 입장한 플레이어 목록을 신입 플레이어한테 전송한다
	if (const auto player = std::dynamic_pointer_cast<Player>(object))
	{
		Protocol::S_SPAWN spawnPkt;

		for (const auto& enterPlayer : _objects | std::views::values)
		{
			if (enterPlayer->IsPlayer() == false)
				continue;

			if (enterPlayer->objectInfo->object_id() == player->objectInfo->object_id())
				continue;
			Protocol::ObjectInfo* playerInfo = spawnPkt.add_players();
			playerInfo->CopyFrom(*enterPlayer->objectInfo);
		}

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
		if (const auto session = player->session.lock())
			session->Send(sendBuffer);
	}

	return success;
}

bool Room::LeaveRoom(ObjectRef object)
{
	if (object == nullptr)
		return false;

	ObjectRef objectRef = object;

	const uint64 objectId = objectRef->objectInfo->object_id();
	bool success = RemoveObject(objectId);

	// 퇴장 사실을 퇴장하는 플레이어에게 알린다
	if (auto player = std::dynamic_pointer_cast<Player>(objectRef))
	{
		const Protocol::S_LEAVE_GAME leaveGamePkt;

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(leaveGamePkt);
		if (const auto session = player->session.lock())
			session->Send(sendBuffer);
	}

	// 퇴장 사실을 룸에 알린다
	{
		Protocol::S_DESPAWN despawnPkt;
		despawnPkt.add_object_ids(objectId);

		const SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(despawnPkt);

		Broadcast(sendBuffer);

		if (auto player = std::dynamic_pointer_cast<Player>(object))
		{
			// RemoveObject() 함수에서 Broadcast 대상에서 빠지기 때문에, 여기서 다시 send
			if (auto session = player->session.lock())
				session->Send(sendBuffer);
		}
	}

	return success;
}

	
bool Room::HandleEnterPlayer(PlayerRef player)
{
	return EnterRoom(player, true);
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	return LeaveRoom(player);
}

void Room::HandleMove(Protocol::C_MOVE pkt)
{
	const uint64 objectId = pkt.info().object_id();
	if (_objects.contains(objectId) == false)
		return;

	// TODO : pkt 정보가 유효한지 확인, 대략적으로 맞는 위치인지
	// 서버에서 검증이 있어야 함

	PlayerRef player = std::dynamic_pointer_cast<Player>(_objects[objectId]);
	player->posInfo->CopyFrom(pkt.info());

	{
		Protocol::S_MOVE movePkt;
		{
			Protocol::PosInfo* info = movePkt.mutable_info();
			info->CopyFrom(pkt.info());
		}

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer);
	}
}

void Room::UpdateTick()
{
	//std::cout << "Update Room" << '\n';

	// TODO

	DoTimer(100, &Room::UpdateTick);
}

RoomRef Room::GetRoomRef()
{
	return std::static_pointer_cast<Room>(shared_from_this());
}

bool Room::AddObject(const ObjectRef& object)
{
	if (_objects.contains(object->objectInfo->object_id()))
		return false;

	ObjectRef objectRef = object; // Add ref
	_objects.insert(std::make_pair(objectRef->objectInfo->object_id(), objectRef));
	objectRef->room.store(GetRoomRef());

	return true;
}

bool Room::RemoveObject(const uint64 objectId)
{
	if (_objects.contains(objectId) == false)
		return false;

	const ObjectRef object = _objects[objectId];
	object->room.store(std::weak_ptr<Room>());

	_objects.erase(objectId);

	return true;
}

void Room::Broadcast(const SendBufferRef& sendBuffer, const uint64 exceptId)
{
	SendBufferRef sendBufferRef = sendBuffer; // ADD Ref

	for (const auto& object : _objects | std::views::values)
	{
		PlayerRef player = std::dynamic_pointer_cast<Player>(object);
		if (player == nullptr)
			continue;

		if (player->objectInfo->object_id() == exceptId)
			continue;

		if (const GameSessionRef session = player->session.lock())
			session->Send(sendBufferRef);
	}
}
