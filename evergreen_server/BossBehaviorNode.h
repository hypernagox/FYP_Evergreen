#pragma once
#include "BehaviorTree.hpp"
#include "PositionComponent.h"

class BossStorageNode
	:public SelectorNode
{
public:
	S_ptr<ContentsEntity> m_cur_target = {};
	int m_pos_idx = 0;
	int m_cur_target_idx = 0;
	float m_cur_target_acc[3]{ 5.f,5.f,5.f };
	bool m_prev_fire = false;
};

class SelectPattern
	: public ConditionNode
{
public:
	// TODO: 성공이라면 근거리공격 이행, 실패라면 다음 패턴으로 이행
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept{}
public:
	float m_probability = .5f;
	float m_origin_prob = .5f;
	int m_count = 0;
	int max_count = 3;
};

class SelectTarget
	:public ActionNode
{
public:
	// TODO: 적당한 유저를 찾는다 찾으면 성공
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
};

class MoveToTarget
	:public ActionNode
{
public:
	// TODO: 찾은 유저로 거리 n 이하까지 계속 이동
	// 이동중이라면 러닝, 조건만족시 성공
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
	float m_accTime = 5.f;
	bool flag = true;
};

// TODO: 리피터 노드가될수도 또는 다양한 근접공격
class MeleeAtack
	:public ActionNode
{
public:
	// TODO: 근거리 공격을 1회 또는 n회 진행한다. 횟수 못채우면 러닝, 다채우면 성공
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:

	float m_accTime = 2.f;
	int m_count = 3;
};

class SelectJumpPoint
	:public ActionNode
{
public:
	// TODO: 사전에 준비해둔 점프 포지션으로 빠르게이동또는 순간이동한다
	// 목적지 도착시 성공
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
	float m_accTime = 1.f;
	bool m_delay_flag = false;
	float m_accTime2 = 1.f;
};

class ShootFireBall
	:public ActionNode
{
public:
	// TODO: 모든 대상을향해 파이어볼을 1회 또는 n회 발사한다. 다하면 성공
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:

	float m_accTime = 2.f;
	int count = 30;
};

class ResetPos
	:public ActionNode
{
public:
	// TODO: 맵 중앙 또는 정해진 위치로 빠르게 이동하여 복귀한다.
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
	bool m_delay_flag = false;
	float m_accTime2 = 3.f;
	bool flag = true;
};

class SetMeteorPos
	:public ActionNode
{
public:
	// TODO: 맵 중앙 위로 날아감
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
	float m_accTime = 1.f;
	bool m_delay_flag = false;
	float m_accTime2 = 1.f;
	
};

class FireMeteor
	:public ActionNode
{
public:
	// TODO: 메테오 N회 발사
	NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const NagiocpX::S_ptr<NagiocpX::ContentsEntity>& awaker)noexcept override;
	virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept {}
public:
	int count = 30;
	float m_accTime = .3f;
	float m_accTime2 = 3.5f;

	std::atomic_bool m_now_meteor = false;
	std::atomic_bool m_hit_catapult = false;
	float m_accCatapultTime = 10.f;
};