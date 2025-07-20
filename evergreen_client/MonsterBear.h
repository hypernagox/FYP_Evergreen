#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterBear : public Monster
{
public:
	Transform* m_transformBody;

	void OnInitialize() override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};