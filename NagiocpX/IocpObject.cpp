#include "NagiocpXPch.h"
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
#include "MoveBroadcaster.h"

namespace NagiocpX
{
	ContentsEntity::ContentsEntity(const uint8_t primary_group_type, const uint8_t detail_type) noexcept
		: m_entity_info{ primary_group_type,detail_type ,IDGenerator::GenerateID() }
		, m_componentSystem{ xnew<ComponentSystemNPC>(m_bIsValid,this) }
	{
		AddComp<ClusterInfoHelper>();
		AddIocpComponent<Queueabler>();
	}

	ContentsEntity::ContentsEntity(Session* const session_) noexcept
		: m_entity_info{ 0,IDGenerator::GenerateID() }
		, m_pSession{ reinterpret_cast<PacketSession* const>(session_) }
		, m_componentSystem{ xnew<ComponentSystem>(m_bIsValid) }
	{
		AddComp<MoveBroadcaster>();
		AddComp<ClusterInfoHelper>();
	}
	
	ContentsEntity::~ContentsEntity() noexcept
	{
		//PrintLogEndl("DESTROY");
		auto b = m_arrIocpComponents;
		const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
		if (m_deleter)
			xdelete<NagoxDeleter>(m_deleter);
		while (e != b) {
			if (const auto iocp_comp = *b++)
				xdelete<IocpComponent>(iocp_comp);
		}
		xdelete<ComponentSystem>(m_componentSystem);
		if (m_pSession) 
			m_pSession->DecRef();
		
	}

	void ContentsEntity::Update(const float dt_) noexcept
	{
		//if (true == m_bNowUpdateFlag.exchange(true, std::memory_order_relaxed))
		//	return;
		m_componentSystem->Update(dt_);
		//m_bNowUpdateFlag.store(false, std::memory_order_release);
	}

	void ContentsEntity::UpdateNonCheck(const float dt_) const noexcept
	{
		m_componentSystem->Update(dt_);
	}

	void ContentsEntity::PostEntityTask(Task&& task_) const noexcept
	{
		const auto ebr_pool = EBRPool<EBRBox<ContentsEntityTask>>::GetEBRPool();
		ebr_pool->Start();
		const auto ebr_node = ebr_pool->PopNode(SharedFromThis(), std::move(task_));
		GlobalEventQueue::PushGlobalEvent(&ebr_node->box_object);
		//::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, ebr_node->box_object.GetOverlappedAddr());
	}

	void ContentsEntity::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		
		S_ptr<ContentsEntity> temp_entity_ptr{ iocpEvent_->PassIocpObject() };

		const auto type = iocpEvent_->GetEventType<uint8_t>();

		if ((uint8_t)EVENT_TYPE::CONTENTS_ENTITY_TASK == type)
		{

			ContentsEntityTask* const entity_task = static_cast<ContentsEntityTask* const>(iocpEvent_);
			entity_task->m_contents_entity_task.ExecuteTask();
			const auto ebr_pool = EBRPool<EBRBox<ContentsEntityTask>>::GetEBRPool();
			ebr_pool->PushNode(EBRBox<ContentsEntityTask>::GetEBRNodeAddress(entity_task));
			ebr_pool->End();
		}
		else
		{
			m_arrIocpComponents[type]->Dispatch(&temp_entity_ptr);
		}
	}

	void ContentsEntity::OnDestroy() noexcept
	{
		const auto cluster_ptr = GetCurCluster();
		if (cluster_ptr)
			cluster_ptr->LeaveAndDestroyEnqueue(GetPrimaryGroupType(), GetObjectID());

		if (nullptr == m_pSession)
			Mgr(FieldMgr)->ReleaseNPC(this);
		else
		{
			if (cluster_ptr)
				m_pSession->OnDisconnected(cluster_ptr);
		}
	}

	void ContentsEntity::ProcessCleanUp() noexcept
	{
		if (m_deleter)
		{
			m_clusterEnterCount = ThreadMgr::NUM_OF_THREADS;
			GetRefCountExternal(this) = 1;
			m_componentSystem->ProcessCleanUp();
			auto b = m_arrIocpComponents;
			const auto e = m_arrIocpComponents + static_cast<int>(IOCP_COMPONENT::END);
			while (e != b) {
				if (const auto iocp_comp = *b++)
					iocp_comp->ProcessCleanUp();
			}
			m_entity_info.SetObjectID4Reuse();

			m_deleter->ProcessDestroy(S_ptr<ContentsEntity>{reinterpret_cast<uint64_t>(this)});
		}
		else
		{
			this->~ContentsEntity();
			Memory::Free(this);
		}
	}

	IocpComponent::IocpComponent(ContentsEntity* const pOwner_, const IOCP_COMPONENT compType) noexcept
		:m_pOwnerEntity{ pOwner_ }, m_iocpCompEvent{ *((uint64_t*)(new ((IocpCompEvent*)&m_iocpCompEvent) IocpCompEvent{compType}))}
	{
		static_assert(sizeof(IocpCompEvent) == sizeof(m_iocpCompEvent));
	}

	IocpComponent::~IocpComponent() noexcept
	{
		GetIocpCompEvent().ReleaseIocpObject();
	}

	void IocpComponent::PostIocpEvent(S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		auto& iocp_comp_event = GetIocpCompEvent();
		
		if (owner_entity && owner_entity->m_count_ptr)
			iocp_comp_event.SetIocpObject(S_ptr<IocpObject>{std::move(*owner_entity)});
		else
			iocp_comp_event.SetIocpObject(S_ptr<IocpObject>{ m_pOwnerEntity });

		GlobalEventQueue::PushGlobalEvent(&iocp_comp_event);
	}

	void IocpComponent::ReserveIocpEvent(const uint64_t tickAfterMs_, S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		auto& iocp_comp_event = GetIocpCompEvent();

		if (owner_entity && owner_entity->m_count_ptr)
			iocp_comp_event.SetIocpObject(S_ptr<IocpObject>{std::move(*owner_entity)});
		else
			iocp_comp_event.SetIocpObject(S_ptr<IocpObject>{ m_pOwnerEntity });

		Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfterMs_, &iocp_comp_event);
	}

	void IocpComponent::SetIncRefEntity(S_ptr<ContentsEntity>&& owner_entity) noexcept
	{
		GetIocpCompEvent().SetIocpObject(std::move(owner_entity));
	}

	S_ptr<ContentsEntity> IocpComponent::PassEntity() noexcept
	{
		return GetIocpCompEvent().PassIocpObject();
	}
}