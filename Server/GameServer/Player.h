#pragma once

class Player
{
public:

	uint64					playerId = 0;
	std::string				name;
	Protocol::PlayerType	playerType = Protocol::PlayerType::PLAYER_TYPE_NONE;
	GameSessionRef			ownerSession;
};

