﻿#include "pch.h"
#include "ThreadManager.h"
#include "CoreTls.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"
#include "JobQueue.h"

using namespace std;

ThreadManager::ThreadManager()
{
	InitTls();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(const std::function<void()>& callback)
{
	LockGuard guard(_lock);

	_threads.emplace_back([callback]()
		{
			InitTls();
			callback();
			DestroyTls();
		});
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	_threads.clear();
}

void ThreadManager::InitTls()
{
	static Atomic<uint32> sThreadId{1};
	LThreadId = sThreadId.fetch_add(1);
}

void ThreadManager::DestroyTls()
{

}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;

		auto jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
			break;

		jobQueue->Execute();
	}
}

void ThreadManager::DistributeReservedJobs()
{
	const uint64 now = ::GetTickCount64();

	GJobTimer->Distribute(now);
}
