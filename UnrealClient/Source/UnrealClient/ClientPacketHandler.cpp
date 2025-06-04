#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "UnrealClient.h"


using namespace std;

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, const int32 len)
{
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	for (auto& Player : pkt.players())
	{
		// TODO : 유저가 가진 player로 뭔가를 함 ㅇㅇ
	}

	// 0 번 케릭터 선택해서 접속한다 가정
	Protocol::C_ENTER_GAME EnterGamePkt;
	EnterGamePkt.set_playerindex(0);
	SEND_PACKET(EnterGamePkt);

	return true;
}

UUnrealClientGameInstance* GetGameInstance()
{
	const auto GameInstance = Cast<UUnrealClientGameInstance>(GWorld->GetGameInstance());
	return GameInstance;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	if (auto GameInstance = GetGameInstance())
	{
		GameInstance->HandleSpawn(pkt);
	}

	return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& session, Protocol::S_LEAVE_GAME& pkt)
{
	if (auto GameInstance = GetGameInstance())
	{
		// TODO : 게임 종료? 로비로? 설정
	}

	return true;
}

bool Handle_S_SPAWN(PacketSessionRef& session, Protocol::S_SPAWN& pkt)
{
	if (auto GameInstance = GetGameInstance())
	{
		GameInstance->HandleSpawn(pkt);
	}

	return true;
}

bool Handle_S_DESPAWN(PacketSessionRef& session, Protocol::S_DESPAWN& pkt)
{
	if (auto GameInstance = GetGameInstance())
	{
		GameInstance->HandleDespawn(pkt);
	}

	return true;
}

bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt)
{
	if (auto GameInstance = GetGameInstance())
	{
		GameInstance->HandleMove(pkt);
	}
	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	std::cout << pkt.msg() << '\n';
	return true;
}

