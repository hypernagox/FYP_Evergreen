#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterFox : public Monster
{
protected:
	udsdx::RiggedMeshRenderer* m_riggedMeshRenderer;

	MonsterHPPanel* m_hpPanel;

public:
	Transform* m_transformBody;

	void OnInitialize() override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};