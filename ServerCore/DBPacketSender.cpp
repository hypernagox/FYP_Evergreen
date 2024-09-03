#include "ServerCorePch.h"
#include "DBPacketSender.h"

namespace ServerCore
{
    void DBPacketSender::Init() noexcept
    {
		m_iocpEvent.SetIocpObject(SharedFromThis<IocpObject>());
		m_sendVec.reserve(1024);
    }

	DBPacketSender::~DBPacketSender() noexcept
	{
	}
  
    void DBPacketSender::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
    {
		const std::vector<S_ptr<SendBuffer>> temp{ std::move(m_sendVec) };

		m_sendVec.reserve(temp.size());

		m_bRegisterSend.store(false, std::memory_order_release);

		if (!m_sendQueue.empty_single() && false == m_bRegisterSend.exchange(true, std::memory_order_relaxed))
			RegisterDBPacket();
    }

    void DBPacketSender::RegisterDBPacket() noexcept
    {
		m_sendQueue.try_flush_single(m_sendVec);

		const auto num = m_sendVec.size();

		if (0 == num)
		{
			m_bRegisterSend.store(false, std::memory_order_release);
			return;
		}

		Vector<WSABUF> wsaBufs;
		wsaBufs.reserve(num);

		for (const auto& sb : m_sendVec)
		{
			wsaBufs.emplace_back(static_cast<const ULONG>(sb->WriteSize()), reinterpret_cast<char* const>(sb->Buffer()));
		}
		
		m_iocpEvent.Init();

		if (SOCKET_ERROR == ::WSASend(m_queryServerSocket, wsaBufs.data(), static_cast<const DWORD>(num), NULL, 0, &m_iocpEvent, nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				// TODO:: 에러처리
			}
		}
    }
}