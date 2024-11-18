#include "ServerCorePch.h"
#include "IocpObject.h"
#include "PacketSession.h"
#include "Service.h"
#include "ContentsComponent.h"
#include "EBR.hpp"
#include "ClusterUpdateQueue.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "FieldMgr.h"
#include "Queueabler.h"

namespace ServerCore
{
	ContentsEntity::ContentsEntity(const uint16_t type_id, const uint8_t obj_type_info) noexcept
		: m_objectCombineID{ CombineObjectID(type_id,IDGenerator::GenerateID()) }
		, m_objTypeInfo{ obj_type_info }
		, m_componentSystem{ xnew<ComponentSystemNPC>(m_bIsValid,this) }
	{
		AddComp<ClusterInfoHelper>();
		AddIocpComponent<Queueabler>();
	}

	ContentsEntity::ContentsEntity(Session* const session_) noexcept
		: m_objectCombineID{ CombineObjectID(0,IDGenerator::GenerateID()) }
		, m_pSession{ reinterpret_cast<PacketSession* const>(session_) }
		, m_componentSystem{ xnew<ComponentSystem>(m_bIsValid) }
	{
		AddComp<MoveBroadcaster>();
		AddComp<ClusterInfoHelper>();
	}
	
	ContentsEntity::~ContentsEntity() noexcept
	{
		std::cout << "DESTROY" << std::endl;
		auto b = m_arrIocpComponents;
		const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
		while (e != b)
		{
			if (const auto iocp_comp = *b++)
				iocp_comp->DecRef();
		}
		xdelete<ComponentSystem>(m_componentSystem);
		if (m_pSession) 
			m_pSession->DecRef();
		
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
		auto& ebr_pool = EBRPool<EBRBox<ContentsEntityTask>>::GetEBRPool();
		ebr_pool.Start();
		const auto ebr_node = ebr_pool.PopNode(SharedFromThis(), std::move(task_));
		::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, &ebr_node->box_object);
	}

	void ContentsEntity::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		const S_ptr<ContentsEntity> temp{ iocpEvent_->PassIocpObject() };
		ContentsEntityTask* const entity_task = static_cast<ContentsEntityTask* const>(iocpEvent_);
		entity_task->m_contents_entity_task.ExecuteTask();
		auto& ebr_pool = EBRPool<EBRBox<ContentsEntityTask>>::GetEBRPool();
		ebr_pool.PushNode(EBRBox<ContentsEntityTask>::GetEBRNodeAddress(entity_task));
		ebr_pool.End();
		//xdelete_sized<ContentsEntityTask>(entity_task, sizeof(ContentsEntityTask));
	}

	void ContentsEntity::OnDestroy() noexcept
	{
		const auto cluster_ptr = GetCurCluster();
		if (cluster_ptr) 
			cluster_ptr->LeaveAndDestroyEnqueue(GetObjectType(), GetObjectID());
		if (const auto queueabler = GetQueueabler())
			queueabler->EnqueueAsyncTaskPushOnly(&ContentsEntity::Destroy, this);
		else
			Destroy();
		if (nullptr == m_pSession)
			Mgr(FieldMgr)->ReleaseNPC(this);
		else
		{
			// GetComp<MoveBroadcaster>()->ReleaseViewList();
			if (cluster_ptr)
				m_pSession->OnDisconnected(cluster_ptr);
		}
	}

	void ContentsEntity::Destroy() noexcept
	{
		//if (m_pSession) 
		//	m_pSession->OnDestroy(); // 나중에, 하트비트 같은거로 튕길 땐 써야할 듯
		{
			auto b = m_arrIocpComponents;
			const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
			while (e != b)
			{
				if (const auto iocp_comp = *b++)
					iocp_comp->OnDestroy();
			}
		}
	}
}