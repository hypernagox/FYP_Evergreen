#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterBoss : public Monster
{
protected:
	udsdx::RiggedMeshRenderer* m_riggedMeshRenderer;

public:

	void OnInitialize() override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};