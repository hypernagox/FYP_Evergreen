#pragma once
#include "ServerComponent.h"

class ForcedMovement
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(ForcedMovement)
public:
	virtual void Update()noexcept override;

public:
	const auto IsForcedMovement()const noexcept { return m_forcedMovementFlag; }
	//void SetForcedMovement(const Nagox::Protocol::s2c_FORCED_MOVEMENT& forced_movement)noexcept;
private:
	bool m_forcedMovementFlag = false;
	Vector3 m_dir = {};
	float m_dist = 0.f;
	float m_speed = 0.f;
};

