#pragma once
#include "ClientNetworkPch.h"

namespace NetHelper
{
	class SendBuffer;
	class Session;
	class s2c_PacketHandler;
	struct PacketHeader;

	template<typename T>requires std::is_enum_v<T>
	constexpr const uint16 etoi(const T eType_)noexcept { return static_cast<const uint16>(eType_); }

	void LogStackTrace()noexcept;

	constexpr static inline const uint64_t GetTimeStampMilliseconds() noexcept {
		return std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(
			std::chrono::steady_clock::now().time_since_epoch()
		).count();
	}

	constexpr static inline const float GetTimeStampSeconds()noexcept {
		return static_cast<const float>(GetTimeStampMilliseconds()) / 1000.f;
	}

	S_ptr<SendBuffer> CreateHeartBeatSendBuffer(const HEART_BEAT eHeartBeatType_) noexcept;

	void Send(S_ptr<SendBuffer> pSendBuffer)noexcept;

	template<typename SessionPtr, typename T>  requires std::convertible_to<SessionPtr, S_ptr<Session>> && !std::derived_from<T, PacketHeader>
	static constexpr void operator<<(SessionPtr&& pSession_, T& pkt_)noexcept
	{
		pSession_->Send(s2c_PacketHandler::MakeSendBuffer(pkt_));
	}

	template<typename SessionPtr, typename T>  requires std::convertible_to<SessionPtr, S_ptr<Session>>
	static constexpr void operator<<(SessionPtr&& pSession_, const T& pkt_)noexcept
	{
		pSession_->Send(pkt_.MakeSendBuffer());
	}

	template<typename T>  requires !std::derived_from<T, PacketHeader>
	static constexpr void Send(T&& pkt_)noexcept
	{
		Send(s2c_PacketHandler::MakeSendBuffer(pkt_));
	}

	template<typename T>
	static constexpr void Send(const T& pkt_)noexcept
	{
		Send(pkt_.MakeSendBuffer());
	}
}