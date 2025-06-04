// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealClientGameInstance.h"

#include "BasePlayer.h"
#include "ClientPacketHandler.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Game/BasePlayer.h"
#include "Interfaces/IPv4/IPv4Address.h"

void UUnrealClientGameInstance::ConnectToGameServer()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(
		TEXT("Stream"), TEXT("Client Socket"));

	FIPv4Address Ip;
	FIPv4Address::Parse(IpAddress, Ip);

	const auto InternetAddr = 
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(Port);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("Connecting to server...")));

	if (Socket->Connect(*InternetAddr))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Connection Success")));

		// Session
		GameServerSession = MakeShared<PacketSession>(Socket);
		GameServerSession->Run();


		// login
		{
			const Protocol::C_LOGIN Pkt;
			SendBufferRef SendBuffer = ClientPacketHandler::MakeSendBuffer(Pkt);
			SendPacket(SendBuffer);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Connection Failed")));
	}
}

void UUnrealClientGameInstance::DisconnectFromGameServer()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	const Protocol::C_LEAVE_GAME LeavePkt;
	SEND_PACKET(LeavePkt);

	/*if (Socket)
	{
		const auto SocketSubSystem = ISocketSubsystem::Get();
		SocketSubSystem->DestroySocket(Socket);
		Socket = nullptr;
	}*/
}

void UUnrealClientGameInstance::HandleRecvPackets()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GameServerSession->HandleRecvPackets();
}

void UUnrealClientGameInstance::SendPacket(SendBufferRef SendBuffer)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GameServerSession->SendPacket(SendBuffer);
}

void UUnrealClientGameInstance::HandleSpawn(
	const Protocol::PlayerInfo& PlayerInfo, const bool IsMine)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	const auto World = GetWorld();
	if (World == nullptr)
		return;

	// 중복 처리 체크
	const uint64 ObjectId = PlayerInfo.object_id();
	if (Players.Contains(ObjectId))
		return;

	const FVector SpawnLocation(PlayerInfo.x(), PlayerInfo.y(), PlayerInfo.z());

	if (IsMine)
	{
		auto* PC = UGameplayStatics::GetPlayerController(this, 0);
		ABasePlayer* Player = Cast<ABasePlayer>(PC->GetPawn());
		if (Player == nullptr)
			return;

		Player->SetPlayerInfo(PlayerInfo);

		MyPlayer = Player;
		Players.Add(ObjectId, Player);
	}
	else
	{
		ABasePlayer* Player = Cast<ABasePlayer>(World->SpawnActor(OtherPlayerClass, &SpawnLocation));
		if (Player == nullptr)
			return;

		Player->SetPlayerInfo(PlayerInfo);
		Players.Add(ObjectId, Player);
	}
}

void UUnrealClientGameInstance::HandleSpawn(
	const Protocol::S_ENTER_GAME& EnterGamePkt)
{
	HandleSpawn(EnterGamePkt.player(), true);
}

void UUnrealClientGameInstance::HandleSpawn(const Protocol::S_SPAWN& SpawnPkt)
{
	for (auto& Player : SpawnPkt.players())
	{
		HandleSpawn(Player, false);
	}
}

void UUnrealClientGameInstance::HandleDespawn(const uint64 ObjectId)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	ABasePlayer** FindActor = Players.Find(ObjectId);
	if (FindActor == nullptr)
		return;

	World->DestroyActor(*FindActor);
}

void UUnrealClientGameInstance::HandleDespawn(
	const Protocol::S_DESPAWN& DespawnPkt)
{
	for (auto& ObjectId : DespawnPkt.object_ids())
	{
		HandleDespawn(ObjectId);
	}
}

void UUnrealClientGameInstance::HandleMove(const Protocol::S_MOVE& MovePkt)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	const uint64 ObjectId = MovePkt.info().object_id();

	ABasePlayer** FindActor = Players.Find(ObjectId);
	if (FindActor == nullptr)
		return;

	ABasePlayer* Player = (*FindActor);
	if (Player->IsMyPlayer())
		return;

	const Protocol::PlayerInfo& Info = MovePkt.info();
	// Player->SetPlayerInfo(Info);
	Player->SetDestInfo(Info);
}
