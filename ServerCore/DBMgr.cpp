#include "ServerCorePch.h"
#include "DBMgr.h"
#include "DBConnectionHandle.h"
#include "ThreadMgr.h"
#include "DBPacket.h"
#include "DBPacketSender.h"
#include "SocketUtils.h"
#include "CoreGlobal.h"
#include "IocpCore.h"
#include "NetAddress.h"
#include "ThreadMgr.h"

namespace ServerCore
{
	DBMgr::DBMgr()
		//: m_packetSender{ MakeShared<DBPacketSender>(SocketUtils::CreateSocket()) }
		//, m_dbHandle{ xnew<DBConnectionHandle>() }
	{
		//m_packetSender->register_cache_shared_core(m_packetSender);
		//m_packetSender->Init();
		//m_queryServerSocket = m_packetSender->GetQueryServerSocket();
		
	}

	DBMgr::~DBMgr()
	{
		//m_cv.notify_all();
		//if (m_queryThread.joinable())
		//	m_queryThread.join();
		//Clear();
		//m_packetSender->Clear();
		//SocketUtils::Close(m_queryServerSocket);
	}

	void DBMgr::Init() noexcept
	{
		//Mgr(CoreGlobal)->GetIocpCore().RegisterIOCP(m_packetSender.get(), 0);
	}

	bool DBMgr::Connect(const std::wstring_view connectionString)
	{
		//if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_environment) != SQL_SUCCESS)
		//	return false;
		//
		//if (::SQLSetEnvAttr(m_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		//	return false;
		//
		//if (m_dbHandle->Connect(m_environment, connectionString) == false)
		//	return false;
		//
		//m_queryThread = std::jthread{ &DBMgr::ExecuteQuery ,this };
		//
		return true;
	}

	void DBMgr::Clear()
	{
		//if (m_environment != SQL_NULL_HANDLE)
		//{
		//	::SQLFreeHandle(SQL_HANDLE_ENV, m_environment);
		//	m_environment = SQL_NULL_HANDLE;
		//}
		//
		//xdelete<DBConnectionHandle>(m_dbHandle);
	}

	bool DBMgr::ConnectQueryServer(const std::wstring_view ip, const uint16_t port)
	{
		//if (INVALID_SOCKET == m_queryServerSocket)
		//	return false;
		//m_netAddr = NetAddress{ ip,port };
		//const auto& addr= m_netAddr.GetSockAddr();
		//if (SOCKET_ERROR == connect(m_queryServerSocket, (const sockaddr* const)&addr, sizeof(addr)))
		//	return false;
		//
		//std::cout << "Connected to Query Server !" << std::endl;

		return true;
	}

	void DBMgr::ExecuteQuery() noexcept
	{
		//LSendBufferChunk = SendBufferMgr::Pop();
		//const bool& bStopFlag = Mgr(ThreadMgr)->GetStopFlagRef();
		//const auto iocpHandle = IocpCore::GetIocpHandleGlobal();
		//Vector<S_ptr<DBEvent>> dbEvents;
		//for (;;)
		//{
		//	{
		//		std::unique_lock lock{ m_mt };
		//		m_cv.wait(lock, [this, &bStopFlag]()noexcept {
		//			return bStopFlag || !m_dbEventQueue.empty_single();
		//			});
		//	}
		//	if (bStopFlag) [[unlikely]]
		//		break;
		//	m_dbEventQueue.try_flush_single(dbEvents);
		//	for (const auto& dbEvent : dbEvents)
		//	{
		//		dbEvent->ExecuteQuery();
		//		//m_dbHandle->Unbind();
		//		::PostQueuedCompletionStatus(iocpHandle, 0, 0, dbEvent->m_dbEvent.GetOverlappedAddr());
		//	}
		//	dbEvents.clear();
		//}
		//Mgr(ThreadMgr)->DestroyTLS();
	}
}