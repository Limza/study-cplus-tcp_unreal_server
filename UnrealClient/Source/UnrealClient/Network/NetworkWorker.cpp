// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetworkWorker.h"

#include "PacketSession.h"
#include "Sockets.h"

FRecvWorker::FRecvWorker(FSocket* Socket, const TSharedPtr<class PacketSession>& Session)
	: Socket(Socket), SessionRef(Session)
{
	Thread = FRunnableThread::Create(this, TEXT("RecvWorkerThread"));
}

FRecvWorker::~FRecvWorker()
{
}

bool FRecvWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("Recv Thread Init")));

	return true;
}

uint32 FRecvWorker::Run()
{
	while (Running)
	{
		// unreal FSocket의 recv는 블록킹 함수기 때문에, 궂이 sleep은 걸 필요가 없음
		if (TArray<uint8> Packet; ReceivePacket(OUT Packet))
		{
			if (const TSharedPtr<PacketSession> Session = SessionRef.Pin())
			{
				Session->RecvPacketQueue.Enqueue(Packet);
			}
		}
	}

	return 0;
}

void FRecvWorker::Exit()
{
	FRunnable::Exit();
}

void FRecvWorker::Destroy()
{
	Running = false;
}

bool FRecvWorker::ReceivePacket(TArray<uint8>& OutPacket) const
{
	// 패킷 헤더 파싱
	constexpr int32 HeaderSize = sizeof(FPacketHeader);
	TArray<uint8> HeaderBuffer;
	HeaderBuffer.AddZeroed(HeaderSize);

	if (ReceiveDesiredBytes(HeaderBuffer.GetData(), HeaderSize) == false)
		return false;

	// Id, Size 추출
	FPacketHeader Header;
	{
		FMemoryReader Reader(HeaderBuffer);
		Reader << Header;
		UE_LOG(LogTemp, Log, TEXT("Recv PacketID : %d, PacketSize : %d"),
			Header.PacketId, Header.PacketSize);
	}

	// 패킷 헤더 복사
	OutPacket = HeaderBuffer;

	// 패킷 내용 파싱
	TArray<uint8> PayloadBuffer;
	const int32 PayloadSize = Header.PacketSize - HeaderSize;
	if (PayloadSize == 0)
		return true;

	OutPacket.AddZeroed(PayloadSize);

	if (ReceiveDesiredBytes(&OutPacket[HeaderSize], PayloadSize))
		return true;

	return true;
}

bool FRecvWorker::ReceiveDesiredBytes(uint8* Results, int32 Size) const
{
	if (uint32 PendingDataSize = 0; 
		Socket->HasPendingData(PendingDataSize) == false || PendingDataSize <= 0)
		return false;

	int32 Offset = 0;

	while (Size > 0)
	{
		int32 NumRead = 0;
		Socket->Recv(Results + Offset, Size, OUT NumRead);
		check(NumRead <= Size);

		if (NumRead <= 0)
			return false;

		Offset += NumRead;
		Size -= NumRead;
	}

	return true;
}

FSendWorker::FSendWorker(FSocket* Socket,
	const TSharedPtr<class PacketSession>& Session)
		: Socket(Socket),  SessionRef(Session)
{
	Thread = FRunnableThread::Create(this, TEXT("SendWorker Thread"));
}

FSendWorker::~FSendWorker()
{
}

bool FSendWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("Send Thread Init")));

	return true;
}

uint32 FSendWorker::Run()
{
	while (Running)
	{
		if (const TSharedPtr<PacketSession> Session = SessionRef.Pin())
		{
			if (SendBufferRef SendBuffer; 
				Session->SendPacketQueue.Dequeue(OUT SendBuffer))
			{
				SendPacket(SendBuffer);
			}
		}
	}

	return 0;
}

void FSendWorker::Exit()
{
	
}

bool FSendWorker::SendPacket(SendBufferRef SendBuffer)
{
	if (SendDesiredBytes(SendBuffer->Buffer(), SendBuffer->WriteSize()) == false)
		return false;

	return true;
}

void FSendWorker::Destroy()
{
	Running = false;
}

bool FSendWorker::SendDesiredBytes(const uint8* Buffer, int32 Size)
{
	while (Size > 0)
	{
		int32 BytesSent = 0;
		if (Socket->Send(Buffer, Size, BytesSent) == false)
			return false;

		Size -= BytesSent;
		Buffer += BytesSent;
	}

	return true;
}
