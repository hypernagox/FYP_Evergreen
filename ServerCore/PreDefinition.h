#pragma once
#include <mutex>
#include <atomic>

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

using c_int8 = const __int8;
using c_int16 = const __int16;
using c_int32 = const __int32;
using c_int64 = const __int64;
using c_uint8 = const unsigned __int8;
using c_uint16 = const unsigned __int16;
using c_uint32 = const unsigned __int32;
using c_uint64 = const unsigned __int64;

namespace ServerCore
{
	// template<typename T>
	// using Atomic = std::atomic<T>;
	using Mutex = std::mutex;
	using CondVar = std::condition_variable;
	using UniqueLock = std::unique_lock<std::mutex>;
	using LockGuard = std::lock_guard<std::mutex>;

	template <typename T>
	struct alignas(64) CacheLineSeperator { alignas(64) T data; };
}

#define size16(val)		(static_cast<int16>(sizeof(val)))
#define size32(val)		(static_cast<int32>(sizeof(val)))
#define len16(arr)		(static_cast<int16>(sizeof(arr)/sizeof(arr[0])))
#define len32(arr)		(static_cast<int32>(sizeof(arr)/sizeof(arr[0])))

//#define _STOMP

#define Mgr(type)	(ServerCore::type::GetInst())

#define SINGLETON(TYPE) \
private: \
friend class Singleton; \
TYPE()noexcept = default; \
~TYPE()noexcept = default;

namespace ServerCore
{
	enum class EVENT_TYPE :uint8
	{
		REGISTER_SEND,
		RECV,
		SEND,
		DISCONNECT,

		CONNECT,
		
		ACCEPT,
		TIMER,
		TASK,
		DB,

		TEMPORARY,

		END,

		TIMER_ROUTINE = 254,
		CONTENTS_ENTITY_TASK = 255
	};

	enum class HEART_BEAT :uint16
	{
		s2c_HEART_BEAT = 12345,
		c2s_HEART_BEAT = 54321
	};

	static constexpr const size_t DEFAULT_MEM_POOL_SIZE = 128;

	static constexpr const int32 NUM_OF_THREADS = 12;
}