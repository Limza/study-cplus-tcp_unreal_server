#pragma once
#include "Types.h"
#include "MemoryPool.h"

template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static Type* Pop(Args&&... args)
	{
#ifdef _STOMP
		const auto ptr = static_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		auto memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		auto memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_Pool.Pop(), s_allocSize));
#endif

		new(memory)Type(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type();

#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_Pool.Push(MemoryHeader::DetachHeader(obj));
#endif
	}

	template<typename... Args>
	static std::shared_ptr<Type> MakeShared(Args&&... args)
	{
		std::shared_ptr<Type> ptr(Pop(std::forward<Args>(args)...), Push);
		return ptr;
	}

private:
	static int32		s_allocSize;
	static MemoryPool	s_Pool;
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_Pool(s_allocSize);