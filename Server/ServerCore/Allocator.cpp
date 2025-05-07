#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

/*----------------------------------
 * Base Allocator
 ----------------------------------*/
void* BaseAllocator::Alloc(const int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}

/*----------------------------------
 * Stomp Allocator
 ----------------------------------*/
void* StompAllocator::Alloc(const int32 size)
{
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const auto dwSize = pageCount * PAGE_SIZE;
	constexpr auto type = MEM_RESERVE | MEM_COMMIT;
	constexpr auto protect = PAGE_READWRITE;
	void* baseAddress = ::VirtualAlloc(nullptr, dwSize, type, protect);

	// overflow ¹æÁö
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	const auto dataStartAddress = static_cast<int8*>(baseAddress) + dataOffset;

	return static_cast<void*>(dataStartAddress);
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);  // NOLINT(performance-no-int-to-ptr)
}

/*----------------------------------
 * Pool Allocator
 ----------------------------------*/
void* PoolAllocator::Alloc(const int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}