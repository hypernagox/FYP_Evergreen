#pragma once
#include <sql.h>
#include <sqlext.h>
#include "DBEvent.h"
#include "DBPacketSender.h"
#include "DBPacket.h"
#include "NetAddress.h"

namespace ServerCore
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
			return nullptr;
			//return m_dbHandle;
		}
		
		void EnqueueDBEvent(S_ptr<DBEvent>&& dbEvent)noexcept
		{
			//m_dbEventQueue.emplace(std::move(dbEvent));
			//m_cv.notify_one();
		}

		void SendDBPacket(S_ptr<SendBuffer>&& pSendBuffer)noexcept
		{
			//m_packetSender->SendDBPacket(std::move(pSendBuffer));
		}
	private:
		void ExecuteQuery()noexcept;
	private:
		//DBConnectionHandle* const m_dbHandle;
		//const S_ptr<DBPacketSender> m_packetSender;
		//MPSCQueue<S_ptr<DBEvent>> m_dbEventQueue;
		//std::mutex m_mt;
		//std::condition_variable m_cv;
		//SQLHENV	m_environment = SQL_NULL_HANDLE;
		//SOCKET m_queryServerSocket = INVALID_SOCKET;
		//std::jthread m_queryThread;
		//NetAddress m_netAddr;
	};
}

template <typename DBEVENT> requires std::derived_from<DBEVENT, ServerCore::DBEvent>
static void RequestQuery(DBEVENT& db)noexcept
{
	ServerCore::S_ptr<ServerCore::DBEvent> dbEvent = ServerCore::MakeShared<DBEVENT>(std::move(db));
	dbEvent->SetEventPtr();
	Mgr(DBMgr)->EnqueueDBEvent(std::move(dbEvent));
}

template <typename DBEVENT> requires std::derived_from<DBEVENT, ServerCore::DBEvent>
static void RequestQuery(DBEVENT&& db)noexcept
{
	ServerCore::S_ptr<ServerCore::DBEvent> dbEvent = ServerCore::MakeShared<DBEVENT>(std::move(db));
	dbEvent->SetEventPtr();
	Mgr(DBMgr)->EnqueueDBEvent(std::move(dbEvent));
}

template <typename T>
static void RequestQueryServer(T&& pkt)noexcept
{
	Mgr(DBMgr)->SendDBPacket(pkt.MakeSendBuffer());
}