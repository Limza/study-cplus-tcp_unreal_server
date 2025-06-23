#pragma once
#include "Protocol.pb.h"
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	Room();
	~Room() override;
	NON_COPYABLE_CLASS(Room);

	bool EnterRoom(ObjectRef object, bool randPos = true);
	bool LeaveRoom(ObjectRef object);

	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	void HandleMove(Protocol::C_MOVE pkt);

public:
	void UpdateTick();

	[[nodiscard]] RoomRef GetRoomRef();

private:
	bool AddObject(const ObjectRef& object);
	bool RemoveObject(uint64 objectId);

	void Broadcast(const SendBufferRef& sendBuffer, uint64 exceptId = 0);

private:
	std::unordered_map<uint64, ObjectRef>	_objects;
};

extern RoomRef GRoom;