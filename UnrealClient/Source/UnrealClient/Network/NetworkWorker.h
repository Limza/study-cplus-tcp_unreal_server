// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"

class FSocket;

struct UNREALCLIENT_API FPacketHeader
{
	FPacketHeader() : PacketSize(0), PacketId(0)
	{	}

	FPacketHeader(const uint16 PacketSize, const uint16 PacketId)
		: PacketSize(PacketSize), PacketId(PacketId)
	{	}

	friend FArchive& operator<<(FArchive& Ar, FPacketHeader& Header)
	{
		Ar << Header.PacketSize;
		Ar << Header.PacketId;
		return Ar;
	}

	uint16 PacketSize;
	uint16 PacketId;
};

class UNREALCLIENT_API FRecvWorker final : public FRunnable
{
public:
	FRecvWorker(FSocket* Socket, const TSharedPtr<class PacketSession>& Session);
	virtual ~FRecvWorker() override;

	virtual bool	Init() override;
	virtual uint32	Run() override;
	virtual void	Exit() override;

	void Destroy();

private:
	bool	ReceivePacket(TArray<uint8>& OutPacket) const;
	bool	ReceiveDesiredBytes(uint8* Results, int32 Size) const;

protected:
	FRunnableThread* Thread = nullptr;
	bool Running = true;
	FSocket* Socket = nullptr;
	TWeakPtr<class PacketSession> SessionRef;
};

class UNREALCLIENT_API FSendWorker final : public FRunnable
{
public:
	FSendWorker(FSocket* Socket, const TSharedPtr<class PacketSession>& Session);
	virtual ~FSendWorker() override;

	virtual bool	Init() override;
	virtual uint32	Run() override;
	virtual void	Exit() override;

	bool SendPacket(SendBufferRef SendBuffer);

	void Destroy();

private:
	bool SendDesiredBytes(const uint8* Buffer, int32 Size);

protected:
	FRunnableThread* Thread = nullptr;
	bool Running = true;
	FSocket* Socket;
	TWeakPtr<class PacketSession> SessionRef;
};