#include "pch.h"
#include "MemoryPool.h"

/* ----------------------------------
 * MemoryPool
 ----------------------------------*/
MemoryPool::MemoryPool(const int32_t allocSize)
    : _allocSize(allocSize)
{
}

void MemoryPool::Push(MemoryHeader* ptr)
{
    ptr->allocSize = 0;
    _pool.deallocate(ptr, _allocSize);
}

MemoryHeader* MemoryPool::Pop()
{
    auto* memory = static_cast<MemoryHeader*>(_pool.allocate(_allocSize));
    return memory;
}





 /*----------------------------------
  * study code
  ---------------------------------- */
///* ----------------------------------
// * MemoryPool
// ----------------------------------*/
//
//MemoryPool::MemoryPool(const int32 allocSize)
//	: _allocSize(allocSize)
//{
//	::InitializeSListHead(&_header);
//}
//
//MemoryPool::~MemoryPool()
//{
//	while (auto* memory = reinterpret_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
//	{
//		::_aligned_free(memory);
//	}
//}
//
//void MemoryPool::Push(MemoryHeader* ptr)
//{
//	ptr->allocSize = 0;
//
//	// Pool에 메모리 반납
//	::InterlockedPushEntrySList(&_header, ptr);
//
//	_allocCount.fetch_sub(1);
//}
//
//MemoryHeader* MemoryPool::Pop()
//{
//	auto memory = reinterpret_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));
//
//	if (memory == nullptr)
//	{
//		// 여분이 없으면 새로 만든다
//		memory = static_cast<MemoryHeader*>(
//			::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
//	}
//	else
//	{
//		ASSERT_CRASH(memory->allocSize == 0);
//	}
//
//	_allocCount.fetch_add(1);
//
//	return memory;
//}
