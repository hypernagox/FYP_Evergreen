#pragma once
#include "NetAddress.h"

namespace NetHelper
{
	class RecvEvent;
	class DisconnectEvent;
	class ConnectEvent;
	class RecvBuffer;
	class SendBuffer;
	class SendEvent;

	/*--------------
		 Session
	---------------*/

	void CALLBACK RecvCompletionRoutine(
		DWORD dwErrorCode,
		DWORD dwNumberOfBytesTransfered,
		LPOVERLAPPED lpOverlapped,
		DWORD dwFlags
	);

	void CALLBACK SendCompletionRoutine(
		DWORD dwErrorCode,
		DWORD dwNumberOfBytesTransfered,
		LPOVERLAPPED lpOverlapped,
		DWORD dwFlags
	);

	class Session
		:public std::enable_shared_from_this<Session>
	{
		friend void CALLBACK RecvCompletionRoutine(
			DWORD dwErrorCode,
			DWORD dwNumberOfBytesTransfered,
			LPOVERLAPPED lpOverlapped,
			DWORD dwFlags
		);
		friend void CALLBACK SendCompletionRoutine(
			DWORD dwErrorCode,
			DWORD dwNumberOfBytesTransfered,
			LPOVERLAPPED lpOverlapped,
			DWORD dwFlags
		);
		friend class NetworkMgr;
	public:
		Session();
		~Session();
		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;
	public:
		void Send(S_ptr<SendBuffer> pSendBuff_)noexcept;
		void Disconnect(std::wstring cause);
		constexpr const uint32_t GetSessionID()const noexcept { return m_iSessionID; }
		const uint32_t GetSessionIDAndAlive()const noexcept { return m_iSessionID * m_bConnected; }
		const bool IsConnected()const noexcept { return m_bConnected; }
		const S_ptr<class PacketSession>& GetCacheSharedFromThis()const noexcept { return m_pCacheSharedFromThis; }
		bool SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept;
	private:
		bool RegisterConnect();
		void ProcessConnect(c_int32 numofBytes_ = 0);

		bool RegisterDisconnect();
		void ProcessDisconnect(c_int32 numofBytes_ = 0);

		void RegisterRecv()noexcept;
		void ProcessRecv(c_int32 numofBytes_);

		void RegisterSend()noexcept;
		void ProcessSend(c_int32 numofBytes_);

		void HandleError(c_int32 errorCode);

		void SetSessionID(c_uint32 sessionID_)noexcept { m_iSessionID = sessionID_; }
		SOCKET GetSocket()const { return m_sessionSocket; }
		HANDLE GetHandle()const noexcept { return  reinterpret_cast<HANDLE>(m_sessionSocket); }
	protected:
		virtual void OnConnected() abstract;
		virtual c_int32 OnRecv(BYTE* const buffer, c_int32 len) abstract;
		virtual void OnSend(c_int32 len) abstract;
		virtual void OnDisconnected()abstract;
	private:
		S_ptr<class PacketSession> m_pCacheSharedFromThis;
		SOCKET m_sessionSocket = INVALID_SOCKET;
		NetAddress m_serverAddr;
		bool m_bConnected = false;
		uint32_t m_iSessionID = 0;
		std::vector<S_ptr<SendBuffer>> m_sendQueue;
		bool m_bIsSendRegistered = false;
	private:
		const U_ptr<RecvBuffer> m_pRecvBuffer;
		const U_ptr<RecvEvent> m_pRecvEvent;
		const U_ptr<ConnectEvent> m_pConnectEvent;
		const U_ptr<DisconnectEvent> m_pDisconnectEvent;
		const U_ptr<SendEvent> m_pSendEvent;

	};
}