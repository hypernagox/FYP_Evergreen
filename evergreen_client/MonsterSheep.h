#pragma once

#include "pch.h"
#include "Monster.h"

class MonsterSheep : public Monster
{
public:
	Transform* m_transformBody;

	void OnInitialize() override;
	void OnDeath() override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};

