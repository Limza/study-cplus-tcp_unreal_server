#pragma once
#include "Job.h"
#include "JobTimer.h"
#include "LockQueue.h"

/* -----------------------------------------------------------------------
 * JobQueue
 -----------------------------------------------------------------------*/
class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	JobQueue() = default;
	virtual ~JobQueue() = default;
	NON_COPYABLE_CLASS(JobQueue);

public:
	void DoAsync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	// NOTE: args 로 참조값 안넘김, 참조값으로 넘겼다가 나중에 Job에서 args 불러올때 없어서
	// 문제(혹은 찾기 힘든 에러)가 생길 가능성이 있다
	// 값으로 넘겨서 명확하게 생명주기를 관리한다
	template<typename T, typename Ret, typename... Args >
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		auto owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	void DoTimer(const uint64 tickAfter, CallbackType&& callback)
	{
		const JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args >
	void DoTimer(const uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		auto owner = std::static_pointer_cast<T>(shared_from_this());
		const JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	void ClearJobs() { _jobs.Clear(); }
	void Execute();
	void Push(JobRef job, bool pushOnly = false);

protected:
	LockQueue<JobRef>	_jobs;
	Atomic<int32>		_jobCount = 0;
};

