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
		void SetTickInterval(const uint16_t tick_ms)noexcept { m_tickInterval = tick_ms; }
		const uint32_t GetAwakeDistance()const noexcept { return m_npcAwakeDistance; }
	public:
		const uint32_t GetAwakerID()const noexcept { return m_curAwakerID; }
		void SetCurAwakerID(const uint32_t awaker_id)noexcept { m_curAwakerID = awaker_id; }
		const auto GetTickTimerDT()const noexcept { return m_tickDT; }
		const float GetFloatDT()const noexcept { return ((float)(m_tickDT)) / 1000.f; }
	public:
		static void BroadcastObjInSight(const XVector<const ContentsEntity*>& temp_vec,const S_ptr<SendBuffer>& send_buff)noexcept;
		static auto& GetTempVecForInsightObj()noexcept { 
			constinit extern thread_local XVector<const class ContentsEntity*>* LXVectorForTempCopy;
			return *LXVectorForTempCopy;
		}
	protected:
		virtual const ServerCore::TIMER_STATE TimerUpdate(const S_ptr<ContentsEntity> awaker)noexcept = 0;
		virtual void ProcessCleanUp()noexcept override;
	private:
		virtual void Dispatch(S_ptr<ContentsEntity>* const owner_entity)noexcept override final;
		const bool TryExecuteTimerInternal(const uint32_t awaker_id)noexcept;
		void Tick(S_ptr<ContentsEntity>* const owner_entity)noexcept;
	protected:
		const uint16_t m_npcAwakeDistance;
		uint8_t m_delay_count = 4; // TODO: 딜레이 매직넘버, 갑자기 몹이 빠르게오는걸 막는 용도임
		NagoxAtomic::Atomic<TIMER_STATE> m_timer_state{ TIMER_STATE::IDLE };
		uint16_t m_tickDT = 0;
		uint16_t m_tickInterval = 200;
		uint32_t m_curAwakerID = 0;
		uint32_t m_lastTickTime = 0;
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
	void SetBTRevaluateInterval(const uint16_t bt_revaluate_interval)noexcept { m_btRevaluateInterval = bt_revaluate_interval; }
	template <typename T = CompositeNode>
	constexpr inline T* const GetRootNode()const noexcept { return static_cast<T* const>(m_rootNode); }
	virtual void ProcessCleanUp()noexcept override;
protected:
	virtual const ServerCore::TIMER_STATE TimerUpdate(const S_ptr<ServerCore::ContentsEntity> awaker)noexcept override;
private:
	uint16_t m_btRevaluateInterval = 2000;
	uint16_t m_accBTRevaluateTime = 0;
	NodeStatus m_prevStatus = NodeStatus::SUCCESS;
	CompositeNode* const m_rootNode;
};

class State 
{
	friend class TickTimerFSM;
protected:
	struct EntityState {
		const uint8_t state;
		EntityState()noexcept = delete;
		template<typename Enum> requires std::is_enum_v<Enum> && (sizeof(uint8_t) == sizeof(Enum))
			constexpr EntityState(const Enum eType)noexcept :state{ (uint8_t)eType } {}
		operator uint8_t()const noexcept { return state; }
	};
public:
	State(const EntityState eState, TickTimerFSM* const fsm) :m_state{ eState }, m_fsm{ fsm } {}
	virtual ~State()noexcept = default;
private:
	virtual const  State::EntityState Update(const float dt, const ComponentSystemNPC* const comp_sys,const S_ptr<ServerCore::ContentsEntity>& awaker)noexcept = 0;
	virtual void Enter(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<ServerCore::ContentsEntity>& awaker)noexcept = 0;
	virtual void Exit(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<ServerCore::ContentsEntity>& awaker)noexcept = 0;
	virtual void Reset()noexcept {}
protected:
	const EntityState m_state;
	TickTimerFSM* const m_fsm;
};

class TickTimerFSM
	:public ServerCore::TickTimer
{
public:
	TickTimerFSM(ServerCore::ContentsEntity* const pOwner_, const uint16_t awakeDist_ = UINT16_MAX)noexcept
		: TickTimer{ pOwner_ ,awakeDist_ }
	{
	}
	virtual ~TickTimerFSM()noexcept { for (const auto state : m_states)ServerCore::xdelete<State>(state.second); }
	template<typename T, typename... Args>
	T* const AddState(Args&&... args)noexcept {
		const auto state = xnew<T>(this, std::forward<Args>(args)...);
		const auto state_key = state->m_state;
		NAGOX_ASSERT(m_states.end() == std::ranges::find_if(m_states, [state_key](const auto& ele)noexcept {
			return ele.first == state_key; }));
		return (T*)m_states.emplace_back(state_key, state).second;
	}
	template<typename T> requires std::is_enum_v<T>
	State* const GetState(const T eType)const noexcept {
		return GetState((uint8_t)eType);
	}
	template<typename T> requires std::is_enum_v<T>
	State* const SetDefaultState(const T eType) noexcept {
		NAGOX_ASSERT(nullptr == m_defaultState);
		m_curState = m_defaultState = GetState(eType);
		NAGOX_ASSERT(m_defaultState && 0 == m_defaultState->m_state);
		return m_defaultState;
	}
protected:
	State* const GetState(const uint8_t eType)const noexcept {
		auto b = m_states.data();
		const auto e = b + m_states.size();
		do {
			if (eType == b->first)return b->second;
		} while (e != ++b);
		return nullptr;
	}
	virtual const ServerCore::TIMER_STATE TimerUpdate(const S_ptr<ServerCore::ContentsEntity> awaker)noexcept override;
	virtual void ProcessCleanUp()noexcept override;
private:
	State* m_curState = nullptr;
	State* m_defaultState = nullptr;
	XVector<std::pair<uint8_t, State* const>> m_states;
};