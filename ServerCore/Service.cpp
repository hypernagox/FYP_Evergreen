#include "ServerCorePch.h"
#include "Service.h"
#include "IocpCore.h"
#include "Session.h"
#include "Listener.h"
#include "SendBufferMgr.h"

namespace ServerCore
{
	Service::Service(const IocpCore& iocpCore_, SERVICE_TYPE eServiceType_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_)
		: m_iocpCore{ iocpCore_ }
		, m_eServiceType{ eServiceType_ }
		, m_netAddr{ addr_ }
		, m_sessionFactory{ std::move(factory_) }
		, m_maxSessionCount{ maxSessionCount_ }
		, m_arrSession{ CreateDynamicSpan<AtomicSessionPtr>(maxSessionCount_ + 1) }
	{
		m_idxQueue.set_capacity(maxSessionCount_ + 1);
		m_id2Index.reserve(maxSessionCount_ * 2);
		for (int i = 1; i <= m_maxSessionCount; ++i)
		{
			m_idxQueue.push(i);
		}
	}

	Service::~Service()
	{
		DestroyDynamicSpan(m_arrSession);
	}

	void Service::CloseService()
	{
		IterateSession([](Session* const p)noexcept {
			if (false == p->Disconnect(L"Bye"))
				p->GetOwnerEntity()->TryOnDestroy();
			});
	}

	S_ptr<Session> Service::CreateSession()noexcept
	{
		auto pSession = m_sessionFactory();
		pSession->SetService(this);
		
		return m_iocpCore.RegisterIOCP(pSession.get(), pSession->GetSessionID()) ? std::move(pSession) : nullptr;
	}

	const bool Service::AddSession(const S_ptr<PacketSession>& pSession_)noexcept
	{
		const ContentsEntity* const pOwnerEntity = pSession_->GetOwnerEntity();
		const uint32_t obj_id = static_cast<c_uint32>(pOwnerEntity->GetObjectID());
		int32 idx;
		if (!m_idxQueue.try_pop(idx))
			return false;
		m_id2Index.emplace(static_cast<c_uint32>(obj_id), static_cast<c_uint16>(idx));
		pSession_->m_serviceIdx = idx;
		m_arrSession[idx].ptr.store(pOwnerEntity->SharedFromThis());
		return true;
	}

	void Service::ReleaseSession(const Session* const pSession_) noexcept
	{
		const int32_t idx = pSession_->m_serviceIdx;
		if (-1 == idx)
			return;
		m_arrSession[idx].ptr.reset();
		m_idxQueue.emplace(idx);
	}

	S_ptr<ContentsEntity> Service::GetSession(const uint64_t sessionID_)const noexcept
	{
		const int32 idx = m_id2Index[static_cast<c_uint32>(sessionID_)];
		auto target = m_arrSession[idx].ptr.load();
		if (target && target->GetObjectID() == sessionID_)
			return target;
		else
			return nullptr;
	}

	void Service::IterateSession(std::function<void(Session*const)> fpIterate_)noexcept
	{
		for (const auto& pSession_ : m_arrSession)
		{
			const auto pSessionEntity = pSession_.ptr.load();
			if (pSessionEntity)
				fpIterate_(const_cast<PacketSession* const>(pSessionEntity->GetSession()));
		}
	}



	ClientService::ClientService(const IocpCore& iocpCore_, NetAddress targetServerAddr_, SessionFactory factory_, c_int32 maxSessionCount_)
		:Service{ iocpCore_,SERVICE_TYPE::CLIENT,targetServerAddr_,std::move(factory_),maxSessionCount_ }
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
		}
		return true;
	}

	void ClientService::CloseService()
	{
		Service::CloseService();
	}



	ServerService::ServerService(const IocpCore& iocpCore_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_)
		: Service{ iocpCore_,SERVICE_TYPE::SERVER,addr_,std::move(factory_),maxSessionCount_ }
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