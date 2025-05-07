#pragma once
#include <stack>

extern thread_local uint32				LThreadId;

// 이번 틱이 완료되는 시간
extern thread_local uint64				LEndTickCount;

// 데드락 프로파일러에서 사용, 쓰레드마다 잡고 있는 락을 추적
extern thread_local std::stack<int32>	LLockStack;

// 현재 쓰레드에서 사용중인 SendBufferChunk
extern thread_local SendBufferChunkRef	LSendBufferChunk;

// 현재 실행하고 있는 JobQueue 추적
extern thread_local class JobQueue*		LCurrentJobQueue;