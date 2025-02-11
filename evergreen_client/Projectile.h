#pragma once
#include "ServerComponent.h"

class Projectile
	:public ServerComponent
{
	CONSTRUCTOR_SERVER_COMPONENT(Projectile)
public:
	virtual void Update()noexcept override;
public:
	Vector3 m_speed = {};
};

