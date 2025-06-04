#pragma once
#include "Protocol.pb.h"

class Room : public std::enable_shared_from_this<Room>
{
public:
	Room();
	virtual ~Room();
	NON_COPYABLE_CLASS(Room);

	bool HandleEnterPlayerLocked(const PlayerRef& player);
	bool HandleLeavePlayerLocked(const PlayerRef& player);
	void HandleMoveLocked(const Protocol::C_MOVE& pkt);

private:
	bool EnterPlayer(const PlayerRef& player);
	bool LeavePlayer(uint64 objectId);

	void Broadcast(const SendBufferRef& sendBuffer, uint64 exceptId = 0);

	USE_LOCK;
private:
	std::unordered_map<uint64, PlayerRef>	_players;
};

extern RoomRef GRoom;