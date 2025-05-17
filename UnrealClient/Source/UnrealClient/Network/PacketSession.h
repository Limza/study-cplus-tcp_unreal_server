// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"

/**
 * 
 */
class UNREALCLIENT_API PacketSession : public TSharedFromThis<PacketSession>
{
public:
	explicit PacketSession(class FSocket* Socket);
	~PacketSession();

	void Run();

	UFUNCTION(BlueprintCallable)
	void HandleRecvPackets();

	void SendPacket(SendBufferRef SendBufferRef);

	void Disconnect();

public:
	class FSocket* Socket;

	TSharedPtr<class FRecvWorker> RecvWorkerThread;
	TSharedPtr<class FSendWorker> SendWorkerThread;

	// GameThread와 NetworkThread가 데이터 주고 받는 공용 큐
	TQueue<TArray<uint8>> RecvPacketQueue;
	TQueue<SendBufferRef> SendPacketQueue;
};
