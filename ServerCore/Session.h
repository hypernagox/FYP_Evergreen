#pragma once
#include "IocpObject.h"
#include "RecvBuffer.h"
#include "ID_Ptr.hpp"
#include "NagoxAtomic.h"

namespace ServerCore
{
	class RecvEvent;
	class Service;
	class DisconnectEvent;
	class ConnectEvent;
	class RecvBuffer;
	class SendBuffer;
	class SendEvent;
	class PacketSession;
	class SectorBehavior;
	class ContentsEntity;

	/*--------------
		 Session
	---------------*/


	class Session
		:public IocpObject
	{
		friend class Service;
		friend class Listener;
		friend class IocpCore;
		friend class PacketSession;
	public:
		using PacketHandleFunc = const bool(*)(const S_ptr<PacketSession>&, const BYTE* const, c_int32);
		static inline void InitializePacketHandleFunc(const PacketHandleFunc* const  sessionPacketHandler_)noexcept { g_sessionPacketHandler = sessionPacketHandler_; }
		static inline const auto GetGlobalSessionPacketHandleFunc()noexcept { return g_sessionPacketHandler; }
	public:
		Session()noexcept;
		Session(const bool bNeedConnect)noexcept;
		virtual ~Session()noexcept;
		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;
		template <typename T> requires std::convertible_to<T, S_ptr<PacketSession>>
		static inline const uint32_t GetID(const T& __restrict pSession_)noexcept { return pSession_->GetSessionID(); }
	public:
		inline void DecRef()const noexcept { RefCountable::DecRef<Session>(); }
		const bool SendQueueEmpty()const noexcept { return m_sendQueue.empty_single(); }
		template <typename S_ptr_SendBuffer> requires std::same_as<std::decay_t<S_ptr_SendBuffer>,S_ptr<SendBuffer>>
		constexpr inline void SendAsync(S_ptr_SendBuffer&& pSendBuff_)const noexcept
		{
			m_sendQueue.emplace(std::forward<S_ptr_SendBuffer>(pSendBuff_));
			TrySend();
		}
		inline void TrySend()const noexcept
		{
			if (false == m_bIsSendRegistered && 
				false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
			{
				const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
				auto& register_send_event = m_pSendEvent->m_registerSendEvent;
				register_send_event.SetIocpObject(SharedFromThis<IocpObject>());
				::PostQueuedCompletionStatus(iocp_handle, 0, 0, register_send_event.GetOverlappedAddr());
			}
		}
		template <typename S_ptr_SendBuffer> requires std::same_as<std::decay_t<S_ptr_SendBuffer>, S_ptr<SendBuffer>>
		constexpr inline void SendOnlyEnqueue(S_ptr_SendBuffer&& pSendBuff_)const noexcept { m_sendQueue.emplace(std::forward<S_ptr_SendBuffer>(pSendBuff_)); }
		bool Connect();

		bool Disconnect(const std::wstring_view cause, S_ptr<PacketSession> move_session)noexcept;

		const uint32_t GetSessionID()const noexcept { return m_pOwnerEntity->GetObjectID(); }
		const auto GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
		const auto GetOwnerEntity()noexcept { return m_pOwnerEntity; }
		//const uint32_t GetSessionIDAndAlive()const noexcept {
		//	std::atomic_thread_fence(std::memory_order_acquire);
		//	return GetOwnerObjectID() * m_bConnected;
		//}
		Service* const GetService()const noexcept { return m_pService; }
		void SetService(Service* const pService_)noexcept { m_pService = pService_; }
		bool SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept;
		//const bool CanRegisterSend()const noexcept { return !m_bIsSendRegistered.load(std::memory_order_relaxed); }
	public:
		void SetNetAddress(NetAddress&& netAddr_)noexcept { m_sessionAddr = netAddr_; }
		const NetAddress& GetAddress()const noexcept{ return m_sessionAddr; }
		SOCKET GetSocket()const noexcept{ return m_sessionSocket; }
		const bool IsConnected()const noexcept { return m_bConnectedNonAtomic; }
		const bool IsConnectedAtomic()const noexcept { return m_bConnected.load(); }
		const bool IsHeartBeatAlive()const noexcept { return m_bHeartBeatAlive; }
		void SetHeartBeat(const bool bHeartBeat_)noexcept { m_bHeartBeatAlive = bHeartBeat_; }
		void SetLastError(c_int32 errCode_)noexcept { m_iLastErrorCode *= errCode_; }
	private:
		HANDLE GetHandle()const noexcept { return reinterpret_cast<HANDLE>(m_sessionSocket); }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;

		void ClearSessionForReuse()noexcept;
	private:
		bool RegisterConnect();
		void ProcessConnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept;


		bool RegisterDisconnect(S_ptr<PacketSession>&& move_session)noexcept;
		void ProcessDisconnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept;

		void RegisterRecv(S_ptr<PacketSession>&& pThisSessionPtr, const RecvBuffer* const pRecvBuffPtr)noexcept;
		void ProcessRecv(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept;

		void RegisterSend(S_ptr<PacketSession>&& pThisSessionPtr)noexcept;
		void ProcessSend(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept;
		void RetrySendAsError(SendEvent* const pSendEvent_)noexcept;

		void HandleError(c_int32 errorCode, S_ptr<PacketSession>&& move_session)noexcept;

		void TryRegisterSend(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept { RegisterSend(std::move(pThisSessionPtr)); }
	protected:
		// 컨텐츠단에서 구현 할 내용들 (오버라이딩)
		virtual void OnConnected() = 0;
		virtual const RecvStatus OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept = 0;
		virtual void OnSend(c_int32 len)noexcept = 0;
		virtual void OnDisconnected(const ServerCore::Cluster* const curCluster_)noexcept = 0;
	private:
		const PadByte pad1 = {};
		ContentsEntity* m_pOwnerEntity;
		const PadByte pad2 = {};
		mutable MPSCQueue<S_ptr<SendBuffer>> m_sendQueue;
		mutable volatile bool m_bIsSendRegistered = false;
		bool m_bConnectedNonAtomic = false;
		SendEvent* const m_pSendEvent;
		SOCKET m_sessionSocket = INVALID_SOCKET;

		Service* m_pService;
		const Us_ptr<DisconnectEvent> m_pDisconnectEvent;
		NetAddress m_sessionAddr;
		const Us_ptr<ConnectEvent> m_pConnectEvent;

		bool m_bConnectedNonAtomicForRecv = false;
		bool m_bTurnOfZeroRecv = true;
		SOCKET m_sessionSocketForRecv;
		RecvEvent* const m_pRecvEvent;
		RecvBuffer* const m_pRecvBuffer;
	private:
		NagoxAtomic::Atomic<bool> m_bConnected{ false };
		volatile bool m_bHeartBeatAlive = true;
		uint16_t m_serviceIdx = 0;
		int32_t m_iLastErrorCode = 1;
	private:
		constexpr static inline void (Session::* const g_sessionLookupTable[etoi(EVENT_TYPE::CONNECT) + 1])(S_ptr<PacketSession>, c_int32)noexcept =
		{
			&Session::TryRegisterSend,
			&Session::ProcessRecv,
			&Session::ProcessSend,
			&Session::ProcessDisconnect,
			&Session::ProcessConnect,
		};

		constinit static inline const PacketHandleFunc* __restrict g_sessionPacketHandler = nullptr;
	};
}

template <typename SESSION>
static inline void operator <<(SESSION&& s, ServerCore::S_ptr<ServerCore::SendBuffer>& b)noexcept
{
	s->SendAsync(b);
}
template <typename SESSION>
static inline void operator <<(SESSION&& s, ServerCore::S_ptr<ServerCore::SendBuffer>&& b)noexcept
{
	s->SendAsync(std::move(b));
}
template <typename SESSION>
static inline void operator +=(SESSION&& s, ServerCore::S_ptr<ServerCore::SendBuffer>& b)noexcept
{
	s->SendOnlyEnqueue(b);
}
template <typename SESSION>
static inline void operator +=(SESSION&& s, ServerCore::S_ptr<ServerCore::SendBuffer>&& b)noexcept
{
	s->SendOnlyEnqueue(std::move(b));
}