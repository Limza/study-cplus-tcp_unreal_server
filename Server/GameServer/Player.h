#pragma once

#include "Creature.h"

class GameSession;
class Room;

class Player : public Creature
{
public:
	Player();
	virtual ~Player();
	NON_COPYABLE_CLASS(Player);

public:
	std::weak_ptr<GameSession> session;
};

