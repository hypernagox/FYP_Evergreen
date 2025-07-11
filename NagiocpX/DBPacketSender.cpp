#include "NagiocpXPch.h"
#include "DBPacketSender.h"

namespace NagiocpX
{
	
    void DBPacketSender::Init() noexcept
    {
		m_sendEvent.SetIocpObject(SharedFromThis<IocpObject>());
    }

	DBPacketSender::~DBPacketSender() noexcept
	{
	}
  
    void DBPacketSender::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
    {
		const auto temp{ std::move(m_sendVec) };

		m_sendVec.reserve(temp.size());

		m_bRegisterSend.store(false, std::memory_order_seq_cst);

		if (!m_sendQueue.empty_single() && false == m_bRegisterSend.exchange(true))
			RegisterDBPacket();
    }

    void DBPacketSender::RegisterDBPacket() noexcept
    {
		extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
		auto& wsabuf_storage = new_view_list_session.GetItemListRef();

		m_sendQueue.try_flush_single(m_sendVec);

		const auto num = static_cast<const DWORD>(wsabuf_storage.size());
		const auto bufs = reinterpret_cast<WSABUF*>(wsabuf_storage.data());

		if (num == 0)
		{
			m_bRegisterSend.store(false, std::memory_order_release);
			return;
		}

		const auto ov_ptr = m_sendEvent.Init();
		
		if (SOCKET_ERROR == ::WSASend(m_queryServerSocket, bufs, static_cast<const DWORD>(num), NULL, 0, ov_ptr, nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				// TODO:: 에러처리
			}
		}
    }
}