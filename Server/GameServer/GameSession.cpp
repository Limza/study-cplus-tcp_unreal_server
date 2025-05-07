#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"

using namespace std;

void GameSession::OnConnected()
{
	GSessionManager.Add(
		std::static_pointer_cast<GameSession>(shared_from_this())
	);
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(
		std::static_pointer_cast<GameSession>(shared_from_this())
	);

	if (_currentPlayer)
	{
		if (const auto room = _room.lock())
			room->DoAsync(&Room::Leave, _currentPlayer);
	}

	_currentPlayer = nullptr;
	_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, const int32 len)
{
	auto session = GetPacketSessionRef();
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(const int32 len)
{
}