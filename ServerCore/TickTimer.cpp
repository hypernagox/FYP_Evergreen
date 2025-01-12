#include "ServerCorePch.h"
#include "TickTimer.h"
#include "TaskTimerMgr.h"
#include "Queueabler.h"
#include "ClusterInfoHelper.h"

namespace ServerCore
{
	TickTimer::TickTimer(ContentsEntity* const pOwner_, const uint16_t awakeDist_) noexcept
		: IocpComponent{ pOwner_,IOCP_COMPONENT::TickTimer }
		, m_npcAwakeDistance{ awakeDist_ }
	{
	}

	const bool TickTimer::TryExecuteTimer(const ContentsEntity* const awaker)noexcept
	{
		if (!IsValid())return false;
		return TryExecuteTimerInternal(awaker);
	}

	void TickTimer::BroadcastObjInSight(const S_ptr<SendBuffer>& send_buff) noexcept
	{
		auto b = m_curObjInSight.data();
		const auto e = b + m_curObjInSight.size();
		while (e != b) { (*b++)->GetSession()->SendAsync(send_buff); }
	}

	void TickTimer::Dispatch(S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		const auto pOwner = GetOwnerEntity();
		const auto queueabler = pOwner->GetQueueabler();
		auto& task_count = queueabler->m_taskCount;
		if (1 == InterlockedIncrement(&task_count))
		{
			Tick(owner_entity);
			if (0 != InterlockedDecrement(&task_count))
			{
				queueabler->Execute(owner_entity);
			}
		}
		else
		{
			queueabler->m_taskQueue.emplace(&TickTimer::Tick, this, (S_ptr<ContentsEntity>*)nullptr);
		}
	}

	const bool TickTimer::TryExecuteTimerInternal(const ContentsEntity* const awaker) noexcept
	{
		const auto eCurState = m_timer_state.load_relaxed();
		if (TIMER_STATE::RUN == eCurState || TIMER_STATE::PREPARE == eCurState)return false;
		const TIMER_STATE ePrevState = m_timer_state.exchange(TIMER_STATE::RUN);
		if (TIMER_STATE::IDLE == ePrevState)
		{
			AwakerInformation(awaker);
			PostIocpEvent();
			return true;
		}
		else if (TIMER_STATE::PREPARE == ePrevState)
		{
			AwakerInformation(awaker);
			ReserveIocpEvent(m_tickInterval);
			return true;
		}
		return false;
	}

	

	void TickTimer::Tick(S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		if (false == IsValid())
		{
			m_curAwaker.reset();
			return;
		}

		//ServerCore::ClusterInfoHelper::FillterSessionEntities(m_curObjInSight, GetOwnerEntity());

		//const TIMER_STATE eCurState = m_curObjInSight.empty() ? TIMER_STATE::IDLE : TimerUpdate();
		const TIMER_STATE eCurState = TimerUpdate();
		std::atomic_thread_fence(std::memory_order_release);
		m_timer_state.store_relaxed(TIMER_STATE::PREPARE);
		std::atomic_thread_fence(std::memory_order_release);
		const TIMER_STATE ePrevState = m_timer_state.exchange(eCurState);
		if (TIMER_STATE::RUN == eCurState && TIMER_STATE::PREPARE == ePrevState)
		{
			ReserveIocpEvent(m_tickInterval, owner_entity);
		}
	}
}

const ServerCore::TIMER_STATE TickTimerBT::TimerUpdate() noexcept
{
	// TODO: 섹터 이동
	const auto pOwnerEntity = GetOwnerEntity();
	//m_curObjInSight = ServerCore::SectorInfoHelper::FillterSessionEntities(pOwnerEntity);
	ServerCore::ClusterInfoHelper::FillterSessionEntities(m_curObjInSight, GetOwnerEntity());
	NodeStatus cur_status;
	const uint64_t cur_time = ::GetTickCount64();
	if (m_curObjInSight.empty())cur_status = NodeStatus::FAILURE;
	else
	{
		const auto owner_comp_sys = static_cast<const ComponentSystemNPC* const>(pOwnerEntity->GetComponentSystem());
	
		const uint16_t diff_time = m_lastTickTime == 0 ? m_tickInterval : static_cast<const uint16_t>(cur_time - m_lastTickTime);
		
		m_accBTRevaluateTime += diff_time;
		if (m_btRevaluateInterval <= m_accBTRevaluateTime)
		{
			m_rootNode->Reset(owner_comp_sys, this);
			m_accBTRevaluateTime = 0;
		}
		m_curBTTimerDT = std::min(static_cast<const float>(diff_time) / 1000.f,0.2f);
		owner_comp_sys->Update(m_curBTTimerDT);
		cur_status = m_rootNode->Tick(owner_comp_sys, this, m_curAwaker);
	}

	if (NodeStatus::FAILURE == cur_status && NodeStatus::FAILURE == m_prevStatus)
	{
		m_prevStatus = NodeStatus::SUCCESS;
		m_lastTickTime = 0;
		return ServerCore::TIMER_STATE::IDLE;
	}
	m_lastTickTime = static_cast<const uint32_t>(cur_time);
	m_prevStatus = cur_status;
	return ServerCore::TIMER_STATE::RUN;
}