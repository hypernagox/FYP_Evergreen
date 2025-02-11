#pragma once
#include "TickTimer.h"

enum class RANGE_MON_STATE :uint8_t 
{
	IDLE,
	CHASE,
	ATTACK,

	END,
};

class RangeMonIdle
	:public State
{
public:
	RangeMonIdle(TickTimerFSM* const fsm)noexcept
		:State{ RANGE_MON_STATE::IDLE,fsm } {
	}
public:
	virtual const  State::EntityState Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Enter(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Exit(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Reset()noexcept {}
private:

};

class RangeMonChase
	:public State
{
public:
	RangeMonChase(TickTimerFSM* const fsm)noexcept
		:State{ RANGE_MON_STATE::CHASE,fsm } {
	}
public:
	virtual const  State::EntityState Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Enter(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Exit(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Reset()noexcept {}
private:

};

class RangeMonAttack
	:public State
{
public:
	RangeMonAttack(TickTimerFSM* const fsm)noexcept
		:State{ RANGE_MON_STATE::ATTACK,fsm } {
	}
public:
	virtual const  State::EntityState Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Enter(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Exit(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override {}
	virtual void Reset()noexcept {}
private:
	float m_acc = 0.f;
};
