#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

/* ----------------------------------
 * Memory
 ---------------------------------- */
Memory::Memory()
{
	int32 size;
	int32 tableIndex = 0;

	for (size = 32; size <= 1024; size += 32)
	{
		auto pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex < size)
			_poolTable[tableIndex++] = pool;
	}

	for (; size <= 2048; size += 128)
	{
		auto pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex < size)
			_poolTable[tableIndex++] = pool;
	}

	for (; size <= 4096; size += 256)
	{
		auto pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex < size)
			_poolTable[tableIndex++] = pool;
	}
}

Memory::~Memory()
{
	for (const MemoryPool* pool : _pools)
		delete pool;

	_pools.clear();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void* Memory::Allocate(const int32 size)
{
	MemoryHeader* header;
	const int32 allocSize = size + static_cast<int32>(sizeof(MemoryHeader));

#ifdef _STOMP
	header = static_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 할당
		header = static_cast<MemoryHeader*>(::malloc(allocSize));
	}
	else
	{
		// 메모리 풀에서 꺼내온다
		header = _poolTable[allocSize]->Pop();
	}
#endif

	return MemoryHeader::AttachHeader(header, allocSize);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Memory::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->allocSize;
	ASSERT_CRASH(allocSize > 0);

#ifdef _STOMP
	StompAllocator::Release(header);
#else
	if (allocSize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 해제
		::free(header);
	}
	else
	{
		// 메모리 풀에 반납한다
		_poolTable[allocSize]->Push(header);
	}
#endif
}
