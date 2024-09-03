#pragma once
#include "IocpObject.h"

namespace ServerCore
{
	class DBPacketSender
		:public IocpObject
	{
	public:
		void Init()noexcept;
		void Clear()noexcept
		{
			m_iocpEvent.ReleaseIocpObject();
		}
		DBPacketSender(const SOCKET s)noexcept :m_queryServerSocket{ s } {}
		virtual ~DBPacketSender()noexcept;
	public:
		HANDLE GetHandle()const noexcept { return reinterpret_cast<HANDLE>(m_queryServerSocket); }
		const SOCKET GetQueryServerSocket()const noexcept { return m_queryServerSocket; }
		void SendDBPacket(S_ptr<SendBuffer> pSendBuffer)noexcept
		{
			m_sendQueue.emplace(std::move(pSendBuffer));
			if (false == m_bRegisterSend.exchange(true, std::memory_order_relaxed))
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				RegisterDBPacket();
			}
		}
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		void RegisterDBPacket()noexcept;
	private:
		MPSCQueue<S_ptr<SendBuffer>> m_sendQueue;
		std::atomic_bool m_bRegisterSend = false;
		std::vector<S_ptr<SendBuffer>> m_sendVec;
		const SOCKET m_queryServerSocket;
		IocpEvent m_iocpEvent{ EVENT_TYPE::DB };
	};
}

