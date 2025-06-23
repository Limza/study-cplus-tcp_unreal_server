// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayer.h"
#include "Protocol.pb.h"
#include "Struct.pb.h"
#include "Engine/GameInstance.h"
#include "UnrealClient.h"
#include "UnrealClientGameInstance.generated.h"

class ABasePlayer;

UCLASS()
class UNREALCLIENT_API UUnrealClientGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ConnectToGameServer();

	UFUNCTION(BlueprintCallable)
	void DisconnectFromGameServer();

	UFUNCTION(BlueprintCallable)
	void HandleRecvPackets();

	void SendPacket(SendBufferRef SendBuffer);

public:
	void HandleSpawn(const Protocol::ObjectInfo& ObjectInfo, const bool IsMine);
	void HandleSpawn(const Protocol::S_ENTER_GAME& EnterGamePkt);
	void HandleSpawn(const Protocol::S_SPAWN& SpawnPkt);

	void HandleDespawn(uint64 ObjectId);
	void HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt);
	void HandleMove(const Protocol::S_MOVE& MovePkt);

public:
	class FSocket* Socket;
	FString IpAddress = TEXT("127.0.0.1");
	int16 Port = 7777;
	TSharedPtr<class PacketSession> GameServerSession;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABasePlayer> OtherPlayerClass;

	ABasePlayer* MyPlayer;
	TMap<uint64, ABasePlayer*> Players;
};
