#include "ServerCorePch.h"
#include "TickTimer.h"
#include "TaskTimerMgr.h"
#include "Queueabler.h"
#include "ClusterInfoHelper.h"
#include "Service.h"

namespace ServerCore
{
	constinit thread_local XVector<const class ContentsEntity*>* LXVectorForTempCopy = nullptr;

	TickTimer::TickTimer(ContentsEntity* const pOwner_, const uint16_t awakeDist_) noexcept
		: IocpComponent{ pOwner_,IOCP_COMPONENT::TickTimer }
		, m_npcAwakeDistance{ awakeDist_ }
	{
	}

	const bool TickTimer::TryExecuteTimer(const uint32_t awaker_id)noexcept
	{
		if (!IsValid())return false;
		return TryExecuteTimerInternal(awaker_id);
	}

	void TickTimer::BroadcastObjInSight(const XVector<const ContentsEntity*>& temp_vec, const S_ptr<SendBuffer>& send_buff) noexcept
	{
		auto b = temp_vec.data();
		const auto e = b + temp_vec.size();
		while (e != b) { (*b++)->GetSession()->SendAsync(send_buff); }
	}

	void TickTimer::ProcessCleanUp() noexcept
	{
		m_delay_count = 4;
		m_lastTickTime = 0;
		m_timer_state.store_relaxed(ServerCore::TIMER_STATE::IDLE);
		m_curAwakerID = 0;
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

	const bool TickTimer::TryExecuteTimerInternal(const uint32_t awaker_id) noexcept
	{
		std::atomic_thread_fence(std::memory_order_acquire);
		const auto eCurState = m_timer_state.load_relaxed();
		if (TIMER_STATE::IDLE != eCurState)return false;
		const TIMER_STATE ePrevState = m_timer_state.exchange(TIMER_STATE::RUN);
		if (TIMER_STATE::IDLE == ePrevState)
		{
			SetCurAwakerID(awaker_id);
			PostIocpEvent();
			return true;
		}
		else if (TIMER_STATE::PREPARE == ePrevState)
		{
			SetCurAwakerID(awaker_id);
			ReserveIocpEvent(m_tickInterval);
			return true;
		}
		return false;
	}

	void TickTimer::Tick(S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		if (false == IsValid())
		{
			return;
		}
		if (0 != m_delay_count)
		{
			m_delay_count = (m_delay_count >> 1);
			m_timer_state.store_relaxed(TIMER_STATE::IDLE);
			return;
		}
		auto& obj_insight = GetTempVecForInsightObj();
		ServerCore::ClusterInfoHelper::FillterSessionEntities(obj_insight, GetOwnerEntity());
		
		const uint64_t cur_time = ::GetTickCount64();
		m_tickDT = m_lastTickTime == 0 ? m_tickInterval : static_cast<const uint16_t>(cur_time - m_lastTickTime);
		
		const TIMER_STATE eCurState = obj_insight.empty() ? TIMER_STATE::IDLE : TimerUpdate(ServerCore::Service::GetMainService()->GetSession(m_curAwakerID));
		const bool run_flag = TIMER_STATE::RUN == eCurState;
		m_lastTickTime = (uint32_t)cur_time * run_flag;
		m_timer_state.store_relaxed(TIMER_STATE::PREPARE);
		std::atomic_thread_fence(std::memory_order_release);
		const TIMER_STATE ePrevState = m_timer_state.exchange(eCurState);

		// 업데이트 하고 PREPARE로만든 그 사이 다른스레드가 들어와서 업데이트 함
		// 그스레드가 업데이트하고 다시 PREPARE로만듬
		// 아까 첫 스레드는 IDLE이 떠서 IDLE로 바꿈
		// 두번째 스레드입장에서는 RUN이 떠서 더 돌려야하는데 익스체인지의 결과로 IDLE이 떠버림
		// 업데이트 해야하는데 못함, 근데 확률이 너무 낮아서 일난 보류
		if (run_flag && TIMER_STATE::PREPARE == ePrevState)
		{
			ReserveIocpEvent(m_tickInterval, owner_entity);
		}
	}
}

const ServerCore::TIMER_STATE TickTimerBT::TimerUpdate(const S_ptr<ServerCore::ContentsEntity> awaker) noexcept
{
	DO_BENCH_GLOBAL_THIS_FUNC;
	// TODO: 섹터 이동
	
	const auto pOwnerEntity = GetOwnerEntity();
	
	const auto owner_comp_sys = static_cast<const ComponentSystemNPC* const>(pOwnerEntity->GetComponentSystem());

	m_accBTRevaluateTime += m_tickDT;

	if (m_btRevaluateInterval <= m_accBTRevaluateTime)
	{
		m_rootNode->Reset(owner_comp_sys, this);
		m_accBTRevaluateTime = 0;
	}
	
	owner_comp_sys->Update(GetFloatDT());

	const NodeStatus cur_status = m_rootNode->Tick(owner_comp_sys, this, awaker);

	if (NodeStatus::FAILURE == cur_status && NodeStatus::FAILURE == m_prevStatus)
	{
		m_prevStatus = NodeStatus::SUCCESS;
		return ServerCore::TIMER_STATE::IDLE;
	}

	m_prevStatus = cur_status;
	return ServerCore::TIMER_STATE::RUN;
}

void TickTimerBT::ProcessCleanUp() noexcept
{
	TickTimer::ProcessCleanUp();
	m_accBTRevaluateTime = 0;
	m_prevStatus = NodeStatus::SUCCESS;
	m_rootNode->Reset(
		(ComponentSystemNPC*)(GetOwnerEntity()->GetComponentSystem())
		, this);
}

const ServerCore::TIMER_STATE TickTimerFSM::TimerUpdate(const S_ptr<ServerCore::ContentsEntity> awaker) noexcept
{
	DO_BENCH_GLOBAL_THIS_FUNC;
	const auto pOwnerEntity = GetOwnerEntity();
	const auto owner_comp_sys = static_cast<const ComponentSystemNPC* const>(pOwnerEntity->GetComponentSystem());
	const float DT = GetFloatDT();
	owner_comp_sys->Update(DT);

	const auto cur_state = m_curState->m_state;

	const auto res_state = m_curState->Update(DT, owner_comp_sys, awaker);

	//if (!cur_state && !res_state)
	//{
	//	m_curState = m_defaultState;
	//	return ServerCore::TIMER_STATE::IDLE;
	//}

	if (cur_state != res_state)
	{
		m_curState->Exit(DT, owner_comp_sys, awaker);
		m_curState = GetState(res_state);
		m_curState->Enter(DT, owner_comp_sys, awaker);
	}

	return ServerCore::TIMER_STATE::RUN;
}

void TickTimerFSM::ProcessCleanUp() noexcept
{
	TickTimer::ProcessCleanUp();
	for (const auto [key, val] : m_states)val->Reset();
}
