#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

// ReSharper disable CppInconsistentNaming
using byte = unsigned char;
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

// ReSharper enable CppInconsistentNaming

template<typename T>
using Atomic		= std::atomic<T>;
using Mutex			= std::mutex;
using CondVar		= std::condition_variable;
using UniqueLock	= std::unique_lock<Mutex>;
using LockGuard		= std::lock_guard<Mutex>;

// shared ptr
#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>;  // NOLINT(bugprone-macro-parentheses)
USING_SHARED_PTR(IocpCore);
USING_SHARED_PTR(IocpObject);
USING_SHARED_PTR(Session);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(ServerService);
USING_SHARED_PTR(ClientService);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(SendBufferChunk);
USING_SHARED_PTR(Job);
USING_SHARED_PTR(JobQueue);

#define size16(val) static_cast<int16>(sizeof(val))
#define size32(val) static_cast<int32>(sizeof(val))
#define len16(arr)	static_cast<int16>(sizeof(arr) / sizeof((arr)[0]))
#define len32(arr)	static_cast<int32>(sizeof(arr) / sizeof((arr)[0]))

#define _STOMP