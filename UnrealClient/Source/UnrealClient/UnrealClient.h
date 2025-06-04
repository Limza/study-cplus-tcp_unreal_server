// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct PacketHeader
{
	uint16 size;
	uint16 id;
};

class SendBuffer : public TSharedFromThis<SendBuffer>
{
public:
	explicit SendBuffer(const int32 bufferSize);
	~SendBuffer() = default;

	BYTE* Buffer() { return _buffer.GetData(); }
	int32 WriteSize() { return _writeSize; }
	int32 Capacity() { return static_cast<int32>(_buffer.Num()); }

	void CopyData(void* data, int32 len);
	void Close(uint32 writeSize);

private:
	TArray<BYTE>	_buffer;
	int32			_writeSize = 0;
};

#define USING_SHARED_PTR(name) using name##Ref = TSharedPtr<class name>;
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(SendBuffer);

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClientGameInstance.h"
#include "ClientPacketHandler.h"

#define SEND_PACKET(Pkt) \
	SendBufferRef SendBuffer = ClientPacketHandler::MakeSendBuffer(Pkt); \
	Cast<UUnrealClientGameInstance>(GWorld->GetGameInstance())->SendPacket(SendBuffer)
