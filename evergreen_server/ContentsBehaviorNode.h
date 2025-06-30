#pragma once
#include "BehaviorTree.hpp"
#include "PositionComponent.h"

class RangeCheckNode
	:public ConditionNode
{
public:
	RangeCheckNode(const uint32_t range) :m_range{ range } {}
public:
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept { m_bReEvaluate = true; }
private:
	bool m_bReEvaluate = false;
	const uint32_t m_range;
};

class ChaseNode
	:public ActionNode
{
public:
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
	
};

class AttackNode
	:public ActionNode
{
public:
    virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;

	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept{}
private:
	const uint32_t m_attack_range = 2;
	float m_accTime = 0.f;
};

class PatrolNode
	:public ActionNode
{
public:
	virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;

	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
	float m_accTime = 0.f;
	float m_accStopTime = 0.f;
	Vector3 m_dir = {};
	Vector3 m_curDest = {};
	bool m_bInit = false;
	bool m_bStop = false;
	float m_accTimeLimit;
	float m_accStopLimit;
	Vector3 m_randPoint[4]{};
};

class MoveNode
	:public ActionNode
{
public:
	virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept
	{
		m_bReEvaluate = true;
	}

private:
	bool m_bReEvaluate = false;

	Vector3 dest_pos;
	Vector3 dir;
	float m_accDist = 0.f;

};
