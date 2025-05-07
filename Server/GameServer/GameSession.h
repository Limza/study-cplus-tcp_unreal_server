#pragma once
#include "Session.h"

class GameSession final : public PacketSession
{
public:
	GameSession() = default;
	~GameSession() override
	{
		std::cout << "GameSession Destructor\n";
	}
	NON_COPYABLE_CLASS(GameSession);

protected:
	void OnConnected() override;
	void OnDisconnected() override;
	void OnRecvPacket(BYTE* buffer, const int32 len) override;
	void OnSend(const int32 len) override;

public:
	Vector<PlayerRef> _players;
	PlayerRef _currentPlayer;
	std::weak_ptr<class Room> _room;
};

