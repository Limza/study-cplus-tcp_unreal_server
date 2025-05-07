#include "pch.h"
#include "Room.h"
#include <ranges>
#include "GameSession.h"
#include "Player.h"

std::shared_ptr<Room> GRoom = std::make_shared<Room>();

void Room::Enter(PlayerRef player)
{
	_players[player->playerId] = player;
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerId);
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (const PlayerRef& player : _players | std::views::values)
	{
		player->ownerSession->Send(sendBuffer);
	}
}