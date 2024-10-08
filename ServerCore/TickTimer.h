#pragma once
#include "BehaviorTree.hpp"

namespace ServerCore
{
	enum class TIMER_STATE : uint8
	{
		IDLE,
		RUN,
		PREPARE,

		END,
	};

	class TickTimer
		:public IocpComponent
	{
	public:
		TickTimer(ContentsEntity* const pOwner_, const uint16_t awakeDist_ = UINT16_MAX) noexcept;
		virtual ~TickTimer()noexcept = default;
	public:
		const bool TryExecuteTimer(const ContentsEntity* const awaker)noexcept;
		void SetTickInterval(const uint32_t tick_ms)noexcept { m_tickInterval = tick_ms; }
		const S_ptr<ContentsEntity>& GetAwaker()const noexcept { return m_curAwaker; }
		const uint32_t GetAwakeDistance()const noexcept { return m_npcAwakeDistance; }
		const auto& GetCurObjInSight()const noexcept { return m_curObjInSight; }
	protected:
		void AwakerInformation(const ContentsEntity* const awaker)noexcept { m_curAwaker = awaker->SharedFromThis(); }
		virtual const ServerCore::TIMER_STATE TimerUpdate()noexcept = 0;
	private:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
		const bool TryExecuteTimerInternal(const ContentsEntity* const awaker)noexcept;
		virtual void OnDestroy()noexcept override;
		void Tick()noexcept;
		template<typename T = TickTimer>
		S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
	protected:
		const uint16_t m_npcAwakeDistance;
		std::atomic<TIMER_STATE> m_timer_state = TIMER_STATE::IDLE;
		uint32_t m_tickInterval = 200;
		S_ptr<ContentsEntity> m_curAwaker;
		Vector<uint32_t> m_curObjInSight;
		IocpEvent* m_timerEvent = xnew<IocpEvent>(EVENT_TYPE::TIMER, SharedFromThis());
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
	auto& GetMutableAwaker()noexcept { return m_curAwaker; }
	template <typename T> requires std::same_as<std::decay_t<T>, ServerCore::S_ptr<ServerCore::ContentsEntity>>
	constexpr inline void SetNewAwaker(T&& new_awaker)noexcept { m_curAwaker = std::forward<T>(new_awaker); }
	inline const float GetBTTimerDT()const noexcept { return  m_curBTTimerDT; }
	void SetBTRevaluateInterval(const uint16_t bt_revaluate_interval)noexcept { m_btRevaluateInterval = bt_revaluate_interval; }
	template <typename T = CompositeNode>
	constexpr inline T* const GetRootNode()const noexcept { return static_cast<T* const>(m_rootNode); }
	template<typename T = TickTimerBT>
	ServerCore::S_ptr<T> SharedFromThis()const noexcept { return ServerCore::S_ptr<T>{this}; }
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