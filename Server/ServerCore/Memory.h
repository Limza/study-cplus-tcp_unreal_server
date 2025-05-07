#pragma once
#include "Allocator.h"

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// placement new, ������ ȣ��
	new(memory)Type(std::forward<Args>(args)...);
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	if constexpr (std::is_class_v<Type>)
	{
		obj->~Type();
		PoolAllocator::Release(obj);
	}
}

template<typename Type, typename... Args>
std::shared_ptr<Type> MakeShared(Args&&... args)
{
	return std::shared_ptr<Type>(xnew<Type>(std::forward<Args>(args)...), xdelete<Type>);
}

/* ----------------------------------
 * Memory
 ---------------------------------- */
class MemoryPool;
class Memory
{
	enum : uint16
	{
		// ~1024���� 32����, ~2048���� 128����, ~4096���� 256 ������ �޸� Ǯ�� �ø���
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void*	Allocate(int32 size);
	void	Release(void* ptr);
private:
	std::vector<MemoryPool*> _pools;

	// �޸� ũ�� <-> �޸� Ǯ
	// 0(1) ������ ã�� ���� ���̺�
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};