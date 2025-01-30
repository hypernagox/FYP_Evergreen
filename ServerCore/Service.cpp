#include "ServerCorePch.h"
#include "Service.h"
#include "IocpCore.h"
#include "Session.h"
#include "Listener.h"
#include "SendBufferMgr.h"
#include "SocketUtils.h"

namespace ServerCore
{
	Service::Service(
		  const IocpCore& iocpCore_
		, SERVICE_TYPE eServiceType_
		, NetAddress addr_
		, SessionFactory factory_
		, const Session::PacketHandleFunc* const global_packet_func,
		  c_int32 maxSessionCount_)
		: m_iocpCore{ iocpCore_ }
		, m_eServiceType{ eServiceType_ }
		, m_netAddr{ addr_ }
		, m_sessionFactory{ std::move(factory_) }
		, m_maxSessionCount{ maxSessionCount_ }
		, m_arrSession{ CreateDynamicSpan<AtomicSessionPtr>(maxSessionCount_ + 1).data()}
		, m_curNumOfSessions{ 0 }
	{
		NAGOX_ASSERT(0 < m_maxSessionCount);
		NAGOX_ASSERT(nullptr != global_packet_func);
		Session::InitializePacketHandleFunc(global_packet_func);
		g_mainService = this;
		m_id2Index.reserve(maxSessionCount_ * 2);
		for (int i = m_maxSessionCount; i >= 1; --i)
		{
			const auto session = CreateSession();
			NAGOX_ASSERT_LOG(nullptr != session, "Fail To Create Session");
			_Post_ _Notnull_ session;
			session->m_serviceIdx = i;
			m_sessionPool.emplace(session);
		}
		std::atomic_thread_fence(std::memory_order_seq_cst);
	}

	Service::~Service()
	{
		const std::span<AtomicSessionPtr> arr{ m_arrSession,(size_t)m_maxSessionCount + 1 };
		DestroyDynamicSpan(arr);
		Session* pSession;
		std::vector<ContentsEntity*> temp;
		while (m_sessionPool.try_pop_single(pSession)) { temp.emplace_back(pSession->GetOwnerEntity()); }
		for (const auto entity : temp)
		{ 
			const auto session = (Session*)entity->GetSession();
			entity->DecRef();
			session->~Session();
			Memory::Free(session);
		}
	}

	void Service::CloseService()
	{
		IterateSession([](Session* const p)noexcept {p->Disconnect(L"Bye", p->SharedFromThis()); });
	}

	Session* const Service::CreateSession()noexcept
	{
		auto pSession = m_sessionFactory();
		pSession->SetService(this);
		return m_iocpCore.RegisterIOCP(pSession, pSession->GetSessionID()) && SocketUtils::SetLinger(pSession->GetSocket(), 1, 0) ? pSession : nullptr;
	}

	const bool Service::AddSession(const S_ptr<PacketSession>& pSession_)noexcept
	{
		const auto arr = m_arrSession;
		const ContentsEntity* const pOwnerEntity = pSession_->GetOwnerEntity();
		const uint32_t obj_id = static_cast<c_uint32>(pOwnerEntity->GetObjectID());
		const uint16_t idx = pSession_->m_serviceIdx;
		if (m_maxSessionCount <= m_curNumOfSessions)return false;
		m_id2Index.emplace(static_cast<c_uint32>(obj_id), static_cast<c_uint16>(idx));
		InterlockedIncrement((LONG*)&m_curNumOfSessions);
		(arr + idx)->ptr.store(pOwnerEntity->SharedFromThis());
		return true;
	}

	void Service::ReleaseSession(const Session* const pSession_) noexcept
	{
		m_arrSession[pSession_->m_serviceIdx].ptr.reset();
	}

	S_ptr<ContentsEntity> Service::GetSession(const uint64_t sessionID_)const noexcept
	{
		const auto arr = m_arrSession;
		const int32 idx = m_id2Index[static_cast<c_uint32>(sessionID_)];
		auto target = (arr + idx)->ptr.load();
		if (target && target->GetObjectID() == sessionID_)
			return target;
		else
			return nullptr;
	}

	void Service::ReturnSession(Session* const pSession) noexcept
	{
		const auto session_limit = m_maxSessionCount - 1;
		PrintLogEndl("Return Session");
		pSession->ClearSessionForReuse();
		const bool re_accept_flag = (session_limit == InterlockedDecrement((LONG*)&m_curNumOfSessions));
		m_sessionPool.emplace(pSession);
		if (re_accept_flag)
		{
			if (SERVICE_TYPE::SERVER == m_eServiceType)
			{
				const auto& listener = static_cast<ServerService*>(this)->GetListener();
				if (false == listener->m_bCanAccept.exchange(true)) {
					listener->RegisterAccept(listener->m_acceptEvent.get());
					std::cout << "Accept available !\n";
				}
			}
		}
	}

	void Service::IterateSession(std::function<void(Session*const)> fpIterate_)noexcept
	{
		// 메인스레드가 센드버퍼가 없어서 OnDisconnect할 때 터질 수 있음
		//for (const auto& pSession_ : m_arrSession)
		//{
		//	const auto pSessionEntity = pSession_.ptr.load();
		//	if (pSessionEntity)
		//		fpIterate_(const_cast<PacketSession* const>(pSessionEntity->GetSession()));
		//}
	}



	ClientService::ClientService(
		  const IocpCore& iocpCore_
		, NetAddress targetServerAddr_
		, SessionFactory factory_
		, const Session::PacketHandleFunc* const global_packet_func
		, c_int32 maxSessionCount_)
		: Service{ iocpCore_,SERVICE_TYPE::CLIENT,targetServerAddr_,std::move(factory_),global_packet_func,maxSessionCount_ }
	{
	}

	ClientService::~ClientService()
	{
	}

	bool ClientService::Start()
	{
		if (false == CanStart())
			return false;
		const int32 sessionCount = GetMaxSessionCount();
		for (int i = 0; i < sessionCount; ++i)
		{
			auto pSession = CreateSession();
			if (false == pSession->Connect())
				return false;
			Sleep(10);
		}
		return true;
	}

	void ClientService::CloseService()
	{
		Service::CloseService();
	}



	ServerService::ServerService(
		  const IocpCore& iocpCore_
		, NetAddress addr_
		, SessionFactory factory_
		, const Session::PacketHandleFunc* const global_packet_func
		, c_int32 maxSessionCount_)
		: Service{ iocpCore_,SERVICE_TYPE::SERVER,addr_,std::move(factory_),global_packet_func,maxSessionCount_ }
		, m_pListener{ MakeShared<Listener>() }
	{
	}

	ServerService::~ServerService()
	{
		m_pListener->FinishServer();
	}

	bool ServerService::Start()
	{
		if (false == CanStart())
			return false;
		if (!m_pListener)
			return false;
		return m_pListener->StartAccept(this);
	}

	void ServerService::CloseService()
	{
		m_pListener->CloseAccept();
		Service::CloseService();
	}
}