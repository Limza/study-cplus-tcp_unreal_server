#pragma once

/*----------------------------------
 * Base Allocator
 ----------------------------------*/
class BaseAllocator
{
public:
	static void*	Alloc(const int32 size);
	static void		Release(void* ptr);
};

/*----------------------------------
 * Stomp Allocator
 ----------------------------------*/
class StompAllocator
{
	enum : uint16 { PAGE_SIZE = 0x1000 };
public:
	static void*	Alloc(const int32 size);
	static void		Release(void* ptr);
};

/*----------------------------------
 * Pool Allocator
 ----------------------------------*/
class PoolAllocator
{
public:
	static void*	Alloc(const int32 size);
	static void		Release(void* ptr);
};

/*----------------------------------
 * STL Allocator
 ----------------------------------*/
template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() = default;

	template<typename Other>
	explicit StlAllocator(const StlAllocator<Other>&) {}

	// ReSharper disable once CppInconsistentNaming
	static T* allocate(const size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	// ReSharper disable once CppInconsistentNaming
	static void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}
};