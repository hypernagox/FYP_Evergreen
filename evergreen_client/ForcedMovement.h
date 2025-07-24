#pragma once
#include "ServerComponent.h"

class ForcedMovement
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(ForcedMovement)
public:
	virtual void Update()noexcept override;
	bool CheckDash()noexcept;
public:
	const auto IsForcedMovement()const noexcept { return m_bIsForcedMovement; }
	void SetForcedMovement(const Vector3& dest_pos)noexcept;
public:
	bool m_bIsForcedMovement = false;
	Vector3 m_dir = {};
	Vector3 m_dest = {};
	float m_dist = 0.f;
	float m_speed = 0.f;
	float acc1 = 5.f;
	float acc2 = 0.f;
	bool speed_flag = false;
};

