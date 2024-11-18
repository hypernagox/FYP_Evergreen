#include "ServerCorePch.h"
#include "TickTimer.h"
#include "TaskTimerMgr.h"
#include "Queueabler.h"

namespace ServerCore
{
	TickTimer::TickTimer(ContentsEntity* const pOwner_, const uint16_t awakeDist_) noexcept
		: IocpComponent{ pOwner_ }
		, m_npcAwakeDistance{ awakeDist_ }
		, m_timerEvent{ xnew<IocpEvent>(EVENT_TYPE::TIMER, SharedFromThis()) }
	{
		// TODO: EBR로 바꾸기
	}
	const bool TickTimer::TryExecuteTimer(const ContentsEntity* const awaker)noexcept
	{
		if (!IsValid())return false;
		return TryExecuteTimerInternal(awaker);
	}

	void TickTimer::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		const auto pOwner = GetOwnerEntity();
		const auto queueabler = pOwner->GetQueueabler();
		auto& task_count = queueabler->m_taskCount;
		if (1 == InterlockedIncrement(&task_count))
		{
			Tick();
			if (0 != InterlockedDecrement(&task_count))
			{
				queueabler->Execute();
			}
		}
		else
		{
			queueabler->m_taskQueue.emplace(&TickTimer::Tick, this);
		}
		pOwner->DecRef();
	}

	const bool TickTimer::TryExecuteTimerInternal(const ContentsEntity* const awaker) noexcept
	{
		const TIMER_STATE ePrevState = m_timer_state.exchange(TIMER_STATE::RUN, std::memory_order_relaxed);
		if (TIMER_STATE::IDLE == ePrevState)
		{
			const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
			AwakerInformation(awaker);
			IncOwnerRef();
			::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_timerEvent);
			return true;
		}
		else if (TIMER_STATE::PREPARE == ePrevState)
		{
			AwakerInformation(awaker);
			IncOwnerRef();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, m_timerEvent);
			return true;
		}
		return false;
	}

	void TickTimer::OnDestroy() noexcept
	{
		const TIMER_STATE ePrevState = m_timer_state.exchange(TIMER_STATE::RUN, std::memory_order_relaxed);
		if (TIMER_STATE::IDLE == ePrevState)
		{
			const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
			IncOwnerRef();
			::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_timerEvent);
			
		}
		else if (TIMER_STATE::PREPARE == ePrevState)
		{
			IncOwnerRef();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, m_timerEvent);
		}
	}

	void TickTimer::Tick() noexcept
	{
		const auto pOwner = GetOwnerEntity();

		if (false == IsValid())
		{
			if (m_timerEvent)
			{
				m_curAwaker.reset();
				xdelete_sized<IocpEvent>(m_timerEvent, sizeof(IocpEvent));
				m_timerEvent = nullptr;
			}
			return;
		}

		//m_curObjInSight = ServerCore::SectorInfoHelper::FillterSessionEntities(pOwner);

		//const TIMER_STATE eCurState = m_curObjInSight.empty() ? TIMER_STATE::IDLE : TimerUpdate();
		const TIMER_STATE eCurState = TimerUpdate();
		m_timer_state.store(TIMER_STATE::PREPARE);
		const TIMER_STATE ePrevState = m_timer_state.exchange(eCurState);
		if (TIMER_STATE::RUN == eCurState && TIMER_STATE::PREPARE == ePrevState)
		{
			IncOwnerRef();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, m_timerEvent);
		}
	}
}

const ServerCore::TIMER_STATE TickTimerBT::TimerUpdate() noexcept
{
	// TODO: 섹터 이동
	const auto pOwnerEntity = GetOwnerEntity();
	//m_curObjInSight = ServerCore::SectorInfoHelper::FillterSessionEntities(pOwnerEntity);
	NodeStatus cur_status;
	const uint64_t cur_time = ::GetTickCount64();
	//if (m_curObjInSight.empty())cur_status = NodeStatus::FAILURE;
	//else
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