#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterBoss : public Monster
{
protected:
	AnimationClip* m_animation;

public:
	void OnInitialize() override;
	void Update(const Time& time, Scene& scene) override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};