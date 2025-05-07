#include "pch.h"

#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"
#include "Room.h"

#include <functional>

using namespace std;

constexpr uint32 iocpTimeoutMs = 10;
constexpr uint64 workerTick = 64;

void DoWorkerJob(const ServerServiceRef& service)
{
	// NOTE : 쓰레드 들은 아래 일감을 순서대로	처리한다
	// 1. 네트워크 처리
	// 2. 글로벌 큐 확인

	while (true)
	{
		LEndTickCount = ::GetTickCount64() + workerTick;

		// 네트워크 입출력 처리 -> 인게임 로직까지(패킷 핸들러)
		service->GetIocpCore()->Dispatch(iocpTimeoutMs);

		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	GRoom->DoTimer(1000, [] { cout << "Hello 1000" << endl; });
	GRoom->DoTimer(2000, [] { cout << "Hello 2000" << endl; });
	GRoom->DoTimer(3000, [] { cout << "Hello 3000" << endl; });
	ClientPacketHandler::Init();

	const ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777)
		, MakeShared<IocpCore>()
		, MakeShared<GameSession>,
		100);

	ASSERT_CRASH(service->Start());

	const uint32 maxThread = std::thread::hardware_concurrency();
	for (uint32 i = 0; i < maxThread; ++i)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	DoWorkerJob(service);

	GThreadManager->Join();
}
