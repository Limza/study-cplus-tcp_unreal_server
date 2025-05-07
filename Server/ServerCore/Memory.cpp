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
		// �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� �Ҵ�
		header = static_cast<MemoryHeader*>(::malloc(allocSize));
	}
	else
	{
		// �޸� Ǯ���� �����´�
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
		// �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� ����
		::free(header);
	}
	else
	{
		// �޸� Ǯ�� �ݳ��Ѵ�
		_poolTable[allocSize]->Push(header);
	}
#endif
}
