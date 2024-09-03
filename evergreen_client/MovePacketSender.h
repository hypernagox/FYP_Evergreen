#pragma once
#include "ServerComponent.h"

class MovePacketSender
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(MovePacketSender)
public:
	virtual void Update()noexcept override;
	void AccDT(const float dt_)noexcept { m_accTime += dt_; }
	void SetSendInterval(const float interval_)noexcept { m_sendInterval = interval_; }
private:
	float m_accTime = 0.f;
	float m_sendInterval = 0.15f;
};

