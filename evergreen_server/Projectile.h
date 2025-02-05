#pragma once
#include "pch.h"
#include "TimerRoutine.h"
#include "IDGenerator.hpp"

class Projectile
	:public ServerCore::TimerRoutine
{
public:
	void SelectObjList(const XVector<const ContentsEntity*>& vec_)noexcept;
private:
	virtual void StartRoutine()noexcept = 0;
	virtual ServerCore::ROUTINE_RESULT Routine()noexcept override;
	virtual void ProcessRemove()noexcept = 0;
public:
	XVector<ServerCore::EntityComp<class Collider>> m_obj_list;
	S_ptr<ContentsEntity> m_owner;
	const uint32_t m_proj_id = (uint32_t)IDGenerator::GenerateID();
	float m_accDist = 0.f;
	Vector3 m_pos = {};
	Vector3 m_speed = {};
	ServerCore::Timer m_timer;

	static constexpr const float MAX_DIST = 10.f * 10.f;
};

class PlayerProjectile
	:public Projectile
{
private:
	virtual void StartRoutine()noexcept override;
	virtual void ProcessRemove()noexcept override;
};

class MonProjectile
	:public Projectile
{
private:
	virtual void StartRoutine()noexcept override;
	virtual void ProcessRemove()noexcept override;
};