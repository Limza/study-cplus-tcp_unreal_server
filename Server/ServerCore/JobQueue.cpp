#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

void JobQueue::Push(JobRef job, bool pushOnly)
{
	// NOTE:아래 count를 먼저 올리고 Push 하는 순서 지켜야 한다
	// job을 먼저 Push하면 Execute에서 jobCount를 sub 할 때 잘못된 결과가 나올 수 있다
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	// 첫번째 Job을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		// tls 에 실행중인 잡큐가 없다면 실행한다
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			Execute();
		}
		else
		{
			// 이미 이 쓰레드에서 잡큐가 돌고 있다면
			// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
			GGlobalQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; ++i)
			jobs[i]->Execute();

		// 남은 일감이 0개라면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		// 이 쓰레드에 일감이 너무 몰려서 계속 실행된다면..
		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;

			// 여유 있는 다름 쓰레드가 실행하도록 GlobalQueue에 Push
			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}
