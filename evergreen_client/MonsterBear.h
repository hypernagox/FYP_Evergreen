#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterBear : public Monster
{
protected:
	std::shared_ptr<udsdx::Material> m_monsterMaterial;
	udsdx::RiggedMeshRenderer* m_riggedMeshRenderer;

	MonsterHPPanel* m_hpPanel;

public:
	Transform* m_transformBody;

	void OnInitialize() override;
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};