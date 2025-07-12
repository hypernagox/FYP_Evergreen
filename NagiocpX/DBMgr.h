#pragma once
#include <sql.h>
#include <sqlext.h>
#include "DBEvent.h"
#include "DBPacketSender.h"
#include "NetAddress.h"

namespace NagiocpX
{
	class DBConnectionHandle;
	class DBPacketSender;

	class DBMgr
		:public Singleton<DBMgr>
	{
		friend class Singleton;
		DBMgr();
		~DBMgr();
	public:
		void Init()noexcept;
		bool Connect(const std::wstring_view connectionString);
		bool ConnectQueryServer(const std::wstring_view ip, const uint16_t port);

		void Clear();

		const DBConnectionHandle* const GetDBHandle()const noexcept { 
			return m_dbHandle;
		}
		
		void EnqueueDBEvent(S_ptr<DBEvent>&& dbEvent)noexcept
		{
			if (IsNotConnectQueryServer())
			{
				return;
			}
			m_dbEventQueue.emplace(std::move(dbEvent));
			m_cv.notify_one();
		}

		void SendDBPacket(S_ptr<SendBuffer>&& pSendBuffer)noexcept
		{
			if (IsNotConnectQueryServer())
			{
				return;
			}
			m_packetSender->SendDBPacket(std::move(pSendBuffer));
		}
		void UnBind()noexcept;
		inline const bool IsNotConnectQueryServer()const noexcept { return SQL_NULL_HANDLE == m_environment; }
	private:
		void ExecuteQuery()noexcept;
	private:
		const S_ptr<DBPacketSender> m_packetSender;
		DBConnectionHandle* const m_dbHandle;
		MPSCQueue<S_ptr<DBEvent>> m_dbEventQueue;
		std::mutex m_mt;
		std::condition_variable m_cv;
		SQLHENV	m_environment = SQL_NULL_HANDLE;
		SOCKET m_queryServerSocket = INVALID_SOCKET;
		std::jthread m_queryThread;
		NetAddress m_netAddr;
	};
}

template <typename DBEVENT> requires std::derived_from<DBEVENT, NagiocpX::DBEvent>
inline static void RequestQuery(DBEVENT& db)noexcept
{
	if (Mgr(DBMgr)->IsNotConnectQueryServer())return;
	NagiocpX::S_ptr<NagiocpX::DBEvent> dbEvent = NagiocpX::MakeShared<DBEVENT>(std::move(db));
	dbEvent->SetEventPtr();
	Mgr(DBMgr)->EnqueueDBEvent(std::move(dbEvent));
}

template <typename DBEVENT> requires std::derived_from<DBEVENT, NagiocpX::DBEvent>
inline static void RequestQuery(DBEVENT&& db)noexcept
{
	if (Mgr(DBMgr)->IsNotConnectQueryServer())return;
	NagiocpX::S_ptr<NagiocpX::DBEvent> dbEvent = NagiocpX::MakeShared<DBEVENT>(std::move(db));
	dbEvent->SetEventPtr();
	Mgr(DBMgr)->EnqueueDBEvent(std::move(dbEvent));
}

template <typename T>
inline static void RequestQueryServer(T&& pkt)noexcept
{
	if (Mgr(DBMgr)->IsNotConnectQueryServer())return;
	constexpr const size_t packetSize = sizeof(T);
	NagiocpX::S_ptr<NagiocpX::SendBuffer> sendBuffer = NagiocpX::SendBufferMgr::Open(packetSize);
	::memcpy(sendBuffer->Buffer(), (char*)&pkt, packetSize);
	sendBuffer->Close(packetSize);
	Mgr(DBMgr)->SendDBPacket(std::move(sendBuffer));
}