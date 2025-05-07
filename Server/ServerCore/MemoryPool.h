#pragma once

#include <memory_resource>

/* ----------------------------------
 * MemoryHeader
 ----------------------------------*/
struct MemoryHeader
{
    explicit MemoryHeader(const int32_t size) : allocSize(size) {}

    static void* AttachHeader(MemoryHeader* header, const int32_t size)
    {
        new (header) MemoryHeader(size); // placement new
        return ++header;
    }

    static MemoryHeader* DetachHeader(void* ptr)
    {
        return static_cast<MemoryHeader*>(ptr) - 1;
    }

    int32_t allocSize;
};

/* ----------------------------------
 * MemoryPool
 ----------------------------------*/
class MemoryPool
{
public:
    explicit MemoryPool(int32_t allocSize);
    ~MemoryPool() = default;
    NON_COPYABLE_CLASS(MemoryPool);

    void Push(MemoryHeader* ptr);
    MemoryHeader* Pop();

private:
    int32_t _allocSize;
    std::pmr::synchronized_pool_resource _pool;
};





/*----------------------------------
 * study code
 ---------------------------------- */
//enum : uint8
//{
//	SLIST_ALIGNMENT = 16,
//};
//
///* ----------------------------------
// * MemoryHeader
// ----------------------------------*/
//DECLSPEC_ALIGN(SLIST_ALIGNMENT)
//struct MemoryHeader : public SLIST_ENTRY
//{
//	// [MemoryHeader][Data]
//	explicit MemoryHeader(const int32 size) : allocSize(size) {}
//
//	static void* AttachHeader(MemoryHeader* header, const int32 size)
//	{
//		new(header)MemoryHeader(size); // placement new
//
//		// 헤더 값만큼 주소 옴긴 후 리턴
//		return ++header;
//	}
//
//	static MemoryHeader* DetachHeader(void* ptr)
//	{
//		// 헤더 값만큼 주소 앞으로 땡겨서, MemoryHeader를 알아낸다
//		MemoryHeader* header = static_cast<MemoryHeader*>(ptr) - 1;
//		return header;
//	}
//
//	int32 allocSize;
//};
//
///* ----------------------------------
// * MemoryHeader
// ----------------------------------*/
//DECLSPEC_ALIGN(SLIST_ALIGNMENT)
//class MemoryPool
//{
//public:
//	// allocSize로 받은 크기만큼의 메모리를 관리한다
//	explicit MemoryPool(const int32 allocSize);
//	~MemoryPool();
//
//	void			Push(MemoryHeader* ptr);
//	MemoryHeader*	Pop();
//
//private:
//	SLIST_HEADER		_header;
//	int32				_allocSize = 0;
//	std::atomic<int32>	_allocCount = 0;
//};

