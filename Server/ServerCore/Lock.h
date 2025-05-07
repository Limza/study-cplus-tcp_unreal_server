#pragma once

#include "Types.h"

/* --------------------------------------
 *	RW SpinLock
 --------------------------------------*/
/*
 * 32 bit 사용
 * 상위 16bit와 하위 16bit의 내용이 달라진다
 * [WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
 * W : WriteFlag (Exclusive Lock Owner ThreadId)
 * R : ReadFlag (Shared Lock Count)
 *
 * 동일 쓰레드가
 * Write lock -> Write lock 허용
 * Write lock -> Read lock 허용
 * Read lock -> Write lock 허용 안함
 */
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000,
	};
public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag{EMPTY_FLAG};
	uint16 _writeCount = 0;
};

/* --------------------------------------
 *	Lock Guards
 --------------------------------------*/
class ReadLockGuard
{
public:
	explicit ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
	{ _lock.ReadLock(_name); }
	~ReadLockGuard() { _lock.ReadUnlock(_name); }
	NON_COPYABLE_CLASS(ReadLockGuard);

private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	explicit WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
	{ _lock.WriteLock(_name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }
	NON_COPYABLE_CLASS(WriteLockGuard);

private:
	Lock& _lock;
	const char* _name;
};