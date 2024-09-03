#include "ServerCorePch.h"
#include "IocpObject.h"
#include "PacketSession.h"
#include "World.h"
#include "Service.h"
#include "ContentsComponent.h"
#include "WorldMgr.h"

namespace ServerCore
{
	ContentsEntity::~ContentsEntity() noexcept
	{
		std::cout << "DESTROY" << std::endl;
		
		xdelete_sized<MoveBroadcaster>(m_moveBroadcaster, sizeof(MoveBroadcaster));
		xdelete<ComponentSystem>(m_componentSystem);
		if (m_pSession) 
			m_pSession->DecRef();
		for (const auto iocp_comp : m_arrIocpComponents)if (iocp_comp)iocp_comp->DecRef();

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
		for (const auto& comp : m_arrIocpComponents)
		{
			if (comp)
				comp->OnDestroy();
		}
		if (nullptr == m_pSession)
			Mgr(WorldMgr)->ReleaseNPC(this);
	}
}