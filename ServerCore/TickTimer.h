#pragma once
#include "BehaviorTree.hpp"
#include "NagoxAtomic.h"

namespace ServerCore
{
	enum class TIMER_STATE : uint8
	{
		IDLE,
		RUN,
		PREPARE,

		END,
	};

	constinit extern thread_local XVector<const class ContentsEntity*>* LXVectorForTempCopy;

	class TickTimer
		:public IocpComponent
	{
	public:
		TickTimer(ContentsEntity* const pOwner_, const uint16_t awakeDist_ = UINT16_MAX) noexcept;
		virtual ~TickTimer()noexcept = default;
	public:
		const bool TryExecuteTimer(const uint32_t awaker_id)noexcept;
		void SetTickInterval(const uint32_t tick_ms)noexcept { m_tickInterval = tick_ms; }
		const uint32_t GetAwakeDistance()const noexcept { return m_npcAwakeDistance; }
	public:
		const uint32_t GetAwakerID()const noexcept { return m_curAwakerID; }
		void SetCurAwakerID(const uint32_t awaker_id)noexcept { m_curAwakerID = awaker_id; }
	public:
		static void BroadcastObjInSight(const XVector<const ContentsEntity*>& temp_vec,const S_ptr<SendBuffer>& send_buff)noexcept;
		static auto& GetTempVecForInsightObj()noexcept { 
			constinit extern thread_local XVector<const class ContentsEntity*>* LXVectorForTempCopy;
			return *LXVectorForTempCopy;
		}
	protected:
		virtual const ServerCore::TIMER_STATE TimerUpdate()noexcept = 0;
	private:
		virtual void Dispatch(S_ptr<ContentsEntity>* const owner_entity)noexcept override final;
		const bool TryExecuteTimerInternal(const uint32_t awaker_id)noexcept;
		void Tick(S_ptr<ContentsEntity>* const owner_entity)noexcept;
	protected:
		const uint16_t m_npcAwakeDistance;
		NagoxAtomic::Atomic<TIMER_STATE> m_timer_state{ TIMER_STATE::IDLE };
		uint32_t m_tickInterval = 200;
		uint32_t m_curAwakerID = 0;
	};
}

class TickTimerBT
	:public ServerCore::TickTimer
{
public:
	TickTimerBT(ServerCore::ContentsEntity* const pOwner_, CompositeNode* const rootNode, const uint16_t awakeDist_ = UINT16_MAX)noexcept
		: TickTimer{ pOwner_ ,awakeDist_ }
		, m_rootNode{ rootNode }
	{}
	~TickTimerBT()noexcept { ServerCore::xdelete<CompositeNode>(m_rootNode); }
public:
	inline const float GetBTTimerDT()const noexcept { return  m_curBTTimerDT; }
	void SetBTRevaluateInterval(const uint16_t bt_revaluate_interval)noexcept { m_btRevaluateInterval = bt_revaluate_interval; }
	template <typename T = CompositeNode>
	constexpr inline T* const GetRootNode()const noexcept { return static_cast<T* const>(m_rootNode); }
	virtual void ProcessCleanUp()noexcept override;
protected:
	virtual const ServerCore::TIMER_STATE TimerUpdate()noexcept override;
private:
	uint32_t m_lastTickTime = 0;
	uint16_t m_btRevaluateInterval = 2000;
	uint16_t m_accBTRevaluateTime = 0;
	float m_curBTTimerDT;
	NodeStatus m_prevStatus = NodeStatus::SUCCESS;
	CompositeNode* const m_rootNode;
};