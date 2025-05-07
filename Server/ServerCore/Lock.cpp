#include "pch.h"
#include "Lock.h"
#include "DeadLockProfiler.h"

using namespace std;

/*
 * 아무도 소유(WriteLock) 및 공유(ReadLock)하고 있지 않을 때,
 * 경합해서 소유권을 얻는다
 * 동일한 쓰레드가 Lock을 다시 소유하는걸 허용한다
 */
void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// lock thread id 검사. 동일 쓰레드라면 통과
	if (const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16; 
		LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	// 내 쓰레드 id 를 상위 16bit에 적는다
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);

	// lock 획득 시도
	const uint64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			if (uint32 expected = EMPTY_FLAG; 
				_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				_writeCount++;
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// Read lock 이 다 풀리기 전에는 Write lock불가능
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공
	if (const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16; 
		LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지 않을 때(Write lock을 안잡은 상태) 경합해서 공유 카운트를 올린다
	const uint64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			// Write lock 이 안 잡혀 있다면 Read lock count 1 증가
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("READ_UNLOCK_ERROR");
}

