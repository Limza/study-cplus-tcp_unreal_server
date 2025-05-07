// ReSharper disable CppInconsistentNaming
#pragma once

#define OUT

/* ------------------------------------------------------------------
		Lock
 -------------------------------------------------------------------- */
#define USE_MANY_LOCKS(count)	Lock _locks[count]
#define USE_LOCK				USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name())
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name())
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/* ------------------------------------------------------------------
		Crash
 -------------------------------------------------------------------- */ 

//#define CRASH(cause)						\
//{											\
//	uint32* crash = nullptr;				\
//	__analysis_assume(crash != nullptr);	\
//	*crash = 0xDEADBEFF;					\
//}

#define CRASH(cause)                                         \
do {                                                         \
    std::cerr << "Crash triggered: " << (cause) << std::endl;\
    assert(false);                                           \
} while (0)

#define ASSERT_CRASH(expr)			\
do {								\
	if (!(expr))					\
	{								\
		CRASH("ASSERT_CRASH");		\
	}								\
} while (0)


/* ------------------------------------------------------------------
		NON_COPYABLE_CLASS
 -------------------------------------------------------------------- */
// ReSharper disable bugprone-macro-parentheses
// 클래스 이름 자체는 식별자 이므로, 괄호 없이도 안전하게 사용 가능
#define NON_COPYABLE_CLASS(ClassName)				\
	private:										\
    ClassName(const ClassName&) = delete;			\
    ClassName& operator=(const ClassName&) = delete;\
    ClassName(ClassName&&) = delete;				\
    ClassName& operator=(ClassName&&) = delete;		\
	/* ReSharper:disable */                    \
    /* IntelliSense workaround */              \
	public:                                     \
    struct __NonCopyableDummy_##ClassName {}
// ReSharper restore bugprone-macro-parentheses