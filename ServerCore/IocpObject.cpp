#include "ServerCorePch.h"
#include "IocpObject.h"
#include "PacketSession.h"
#include "World.h"
#include "Service.h"
#include "ContentsComponent.h"
#include "WorldMgr.h"
#include "SectorInfoHelper.h"

namespace ServerCore
{
	ContentsEntity::ContentsEntity(const uint16_t type_id, const uint8_t obj_type_info) noexcept
		: m_objectCombineID{ CombineObjectID(type_id,IDGenerator::GenerateID()) }
		, m_objTypeInfo{ obj_type_info }
		, m_componentSystem{ xnew<ComponentSystemNPC>(m_bIsValid,this) }
	{
		AddComp<SectorInfoHelper>();
	}

	ContentsEntity::ContentsEntity(Session* const session_) noexcept
		: m_objectCombineID{ CombineObjectID(0,IDGenerator::GenerateID()) }
		, m_pSession{ reinterpret_cast<PacketSession* const>(session_) }
		, m_componentSystem{ xnew<ComponentSystem>(m_bIsValid) }
	{
		AddComp<MoveBroadcaster>();
		AddComp<SectorInfoHelper>();
	}
	
	ContentsEntity::~ContentsEntity() noexcept
	{
		std::cout << "DESTROY" << std::endl;
		
		xdelete<ComponentSystem>(m_componentSystem);
		if (m_pSession) 
			m_pSession->DecRef();
		auto b = m_arrIocpComponents;
		const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
		while (e != b)
		{
			if (const auto iocp_comp = *b++)
				iocp_comp->DecRef();
		}
	}

	void ContentsEntity::Update(const float dt_) noexcept
	{
		if (true == m_bNowUpdateFlag.exchange(true, std::memory_order_relaxed))
			return;
		m_componentSystem->Update(dt_);
		m_bNowUpdateFlag.store(false, std::memory_order_release);
	}

	void ContentsEntity::UpdateNonCheck(const float dt_) const noexcept
	{
		m_componentSystem->Update(dt_);
	}

	void ContentsEntity::PostEntityTask(Task&& task_) const noexcept
	{
		::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, xnew<ContentsEntityTask>(SharedFromThis(), std::move(task_)));
	}

	void ContentsEntity::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		const S_ptr<ContentsEntity> temp{ iocpEvent_->PassIocpObject() };
		ContentsEntityTask* const entity_task = static_cast<ContentsEntityTask* const>(iocpEvent_);
		entity_task->m_contents_entity_task.ExecuteTask();
		xdelete_sized<ContentsEntityTask>(entity_task, sizeof(ContentsEntityTask));
	}

	void ContentsEntity::OnDestroy() noexcept
	{
		const auto sector = GetCombinedSectorInfo();
		const auto sector_ptr = sector.GetPtr();
		if (m_pSession)
			m_pSession->OnDisconnected(sector);
		if (sector_ptr)
			sector_ptr->LeaveAndDestroyEnqueue(GetObjectType(), GetObjectID());
		else
			Destroy();
	}

	void ContentsEntity::Destroy() noexcept
	{
		//if (m_pSession) 
		//	m_pSession->OnDestroy(); // 나중에, 하트비트 같은거로 튕길 땐 써야할 듯
		auto b = m_arrIocpComponents;
		const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
		while (e != b)
		{
			if (const auto iocp_comp = *b++)
				iocp_comp->OnDestroy();
		}
		if (nullptr == m_pSession)
			Mgr(WorldMgr)->ReleaseNPC(this);
	}
}