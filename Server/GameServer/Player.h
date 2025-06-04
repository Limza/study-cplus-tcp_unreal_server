#pragma once

class GameSession;
class Room;

class Player : public std::enable_shared_from_this<Player>
{
public:
	Player();
	virtual ~Player();
	NON_COPYABLE_CLASS(Player);

public:
	Protocol::PlayerInfo* playerInfo;
	std::weak_ptr<GameSession> session;

public:
	std::atomic<std::weak_ptr<Room>> room;
};

