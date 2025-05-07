#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

void JobTimer::Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner,
	JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	auto jobData = ObjectPool<JobData>::Pop(owner, job);

	WRITE_LOCK;
	_items.push(TimerItem{.executeTick = executeTick, .jobData = jobData });
}

void JobTimer::Distribute(uint64 now)
{
	// 한번에 1 쓰레드만 통과. 여러개의 쓰레드를 통과시키면
	// 정말 낮은 확률로, 일감의 순서가 꼬일 수 있음
	if (_distributing.exchange(true) == true)
		return;

	Vector<TimerItem> items;
	{
		WRITE_LOCK;
		while (_items.empty() == false)
		{
			const auto& timerItem = _items.top();
			if (now < timerItem.executeTick )
				break;

			items.push_back(timerItem);
			_items.pop();
		}
	}

	for (const TimerItem& item: items)
	{
		if (JobQueueRef owner = item.jobData->owner.lock())
			owner->Push(item.jobData->job);

		ObjectPool<JobData>::Push(item.jobData);
	}

	_distributing.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while (_items.empty() == false)
	{
		const auto& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem.jobData);
		_items.pop();
	}
}
