#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	constinit extern thread_local uint32_t LRandSeed;

	class ContentsEntity;
	class SendBuffer;
	class SessionManageable;
	class Session;
	class PacketSession;
	struct PacketHeader;
	
	template <typename T>
	class S_ptr;

	template <typename T> requires std::is_enum_v<T>
	constexpr inline const uint16 etoi(const T eType_)noexcept { return static_cast<const uint16>(eType_); }

	template <typename T> requires std::is_enum_v<T>
	constexpr inline const uint8_t etoi_u8(const T eType_)noexcept { return static_cast<const uint8_t>(eType_); }

	template <typename T> requires std::is_fundamental_v<T> || std::is_pointer_v<T>
	constexpr inline T rcast(const T val)noexcept { return val; }

	template <typename T> requires (!std::is_fundamental_v<std::decay_t<T>>)
	constexpr inline T rcast(T&& val)noexcept { return std::forward<T>(val); }

	template <typename T>
	static inline const bool compareExchange(T* volatile* const ptr, T** const old_ptr, const T* const new_ptr)noexcept
	{
		return std::atomic_compare_exchange_strong_explicit(
			reinterpret_cast<volatile std::atomic_llong* const>(ptr),
			reinterpret_cast<long long* const>(old_ptr),
			reinterpret_cast<const long long>(new_ptr),
			std::memory_order_relaxed,
			std::memory_order_relaxed
		);
	}
	static inline const uint64_t CombineObjectID(const uint16_t type_id, const uint64_t obj_id)noexcept {
		return (static_cast<const uint64_t>(type_id) << 48) | obj_id;
	}
	static inline const uint64_t GetObjectID(const uint64_t combine_id)noexcept { return combine_id & 0xFFFFFFFFFFFF; }
	static inline const uint16_t GetObjectType(const uint64_t combine_id)noexcept { return static_cast<const uint16_t>(combine_id >> 48); }
	static inline const uint32_t my_rand()noexcept {
		constinit extern thread_local uint32_t LRandSeed;
		LRandSeed = LRandSeed * 1103515245 + 12345;
		return (LRandSeed >> 16) & RAND_MAX;
	}
	
	S_ptr<ContentsEntity> GetSession(const uint64_t sessionID_)noexcept;

	void SendPacket(const uint64_t target_session_id, S_ptr<SendBuffer> pSendBuffer)noexcept;

	void PrintError(const char* const msg, const int err_no)noexcept;
	
	void LogStackTrace()noexcept;

	const uint32 GetCurThreadIdx()noexcept;

	template<typename T>
	class AllocateSharedSizeTrait
	{
	public:
		std::size_t* actual_size_ptr = nullptr;

		using value_type = T;

		constexpr AllocateSharedSizeTrait()noexcept {}

		template<typename Other>
		AllocateSharedSizeTrait(const AllocateSharedSizeTrait<Other>& al) :actual_size_ptr{ al.actual_size_ptr } {}

		constexpr T* const allocate(const size_t size)noexcept
		{
			//NAGOX_ASSERT(actual_size_ptr);
			(*actual_size_ptr) = size * sizeof(T);
			return static_cast<T* const>(::malloc(size * sizeof(T)));
		}

		constexpr void deallocate(T* const ptr, const size_t count)noexcept
		{
			::free(ptr);
		}
	};

	template <typename T>
	constexpr const std::size_t AllocateSharedActualSize()noexcept
	{
		std::size_t temp = 0;
		AllocateSharedSizeTrait<T> dummyAllocator;
		dummyAllocator.actual_size_ptr = &temp;
		const auto dummy = std::allocate_shared<T>(dummyAllocator);
		ASSERT_CRASH(0 != temp);
		return temp;
	}

	constexpr static inline const uint64_t GetTimeStampMilliseconds() noexcept {
		return std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(
			std::chrono::steady_clock::now().time_since_epoch()
		).count();
	}

	constexpr static inline const uint64_t GetSystemTimeStampMilliseconds() noexcept {
		return std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();
	}

	constexpr static inline const float GetTimeStampSeconds()noexcept {
		return static_cast<const float>(GetTimeStampMilliseconds()) / 1000.f;
	}

	S_ptr<SendBuffer> CreateHeartBeatSendBuffer(const HEART_BEAT eHeartBeatType_) noexcept;

	//template <typename T, typename U>
	//struct SendPairData
	//{
	//	T first;
	//	U second;
	//};
	//
	//template<typename T> requires !std::derived_from<T, PacketHeader>
	//static constexpr SendPairData<S_ptr<Session>, S_ptr<SendBuffer>> operator + (S_ptr<Session> pSendSession_, T&& pkt_)noexcept
	//{
	//	return { std::move(pSendSession_), c2s_PacketHandler::MakeSendBuffer(pkt_) };
	//}
	//
	//template<typename T> requires !std::derived_from<T, PacketHeader>
	//static constexpr SendPairData<S_ptr<SendBuffer>, uint64> operator - (T&& pkt_, const S_ptr<Session>& pSendSession_)noexcept
	//{
	//	return { c2s_PacketHandler::MakeSendBuffer(pkt_),ServerCore::Session::GetID(pSendSession_) };
	//}
	//
	//template<typename RoomPtr> requires std::convertible_to<RoomPtr, S_ptr<SessionManageable>> || std::convertible_to<RoomPtr, SessionManageable*>
	//static constexpr void operator<<(RoomPtr&& pRoom_, SendPairData<S_ptr<Session>, S_ptr<SendBuffer>>&& session_msg)noexcept
	//{
	//	pRoom_->SendEnqueue(std::move(session_msg.first), std::move(session_msg.second));
	//}
	//
	//template<typename RoomPtr> requires std::convertible_to<RoomPtr, S_ptr<SessionManageable>> || std::convertible_to<RoomPtr, SessionManageable*>
	//static constexpr void operator<<(RoomPtr&& pRoom_, SendPairData<S_ptr<SendBuffer>, uint64>&& msg_exceptNum)noexcept
	//{
	//	pRoom_->BroadCastEnqueue(std::move(msg_exceptNum.first), std::move(msg_exceptNum.second));
	//}
	//
	//template<typename RoomPtr, typename T> requires (std::convertible_to<RoomPtr, S_ptr<SessionManageable>> || std::convertible_to<RoomPtr, SessionManageable*>) && !std::derived_from<T, PacketHeader>
	//static constexpr void operator<<(RoomPtr&& pRoom_, T&& pkt_)noexcept
	//{
	//	pRoom_->BroadCastEnqueue(c2s_PacketHandler::MakeSendBuffer(pkt_));
	//}
	//
	//template<typename SessionPtr, typename T> requires (std::convertible_to<SessionPtr, S_ptr<Session>> || std::convertible_to<SessionPtr, Session*>) && !std::derived_from<T, PacketHeader>
	//inline static constexpr void operator<<(SessionPtr&& pSession_, T&& pkt_)noexcept
	//{
	//	pSession_->SendAsync(c2s_PacketHandler::MakeSendBuffer(pkt_));
	//}
	//
	//template<typename SessionPtr> requires (std::convertible_to<SessionPtr, S_ptr<Session>> || std::convertible_to<SessionPtr, Session*>)
	//inline static constexpr void operator<<(SessionPtr&& pSession_, S_ptr<SendBuffer> pkt_)noexcept
	//{
	//	pSession_->SendAsync(std::move(pkt_));
	//}
}
