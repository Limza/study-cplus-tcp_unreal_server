// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Protocol.pb.h"
#include "GameFramework/Character.h"
#include "BasePlayer.generated.h"

UCLASS()
class UNREALCLIENT_API ABasePlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ABasePlayer();
	virtual ~ABasePlayer();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	[[nodiscard]] bool IsMyPlayer();
	[[nodiscard]] Protocol::PosInfo* GetPlayerInfo() const { return PlayerInfo; }
	[[nodiscard]] Protocol::MoveState GetMoveState() const { return PlayerInfo->state(); }

	void SetPlayerInfo(const Protocol::PosInfo& Info);
	void SetDestInfo(const Protocol::PosInfo& Info) const;
	void SetMoveState(const Protocol::MoveState& State) const;

protected:
	Protocol::PosInfo* PlayerInfo; // 현재 위치
	Protocol::PosInfo* DestInfo; // 목적지
};