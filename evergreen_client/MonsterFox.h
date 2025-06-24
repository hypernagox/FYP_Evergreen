#pragma once

#include "pch.h"
#include "Monster.h"

using namespace udsdx;

class MonsterFox : public Monster
{
protected:
	std::shared_ptr<udsdx::Material> m_monsterMaterial;
	udsdx::RiggedMeshRenderer* m_riggedMeshRenderer;

	MonsterHPPanel* m_hpPanel;

public:
	Transform* m_transformBody;
	MonsterFox(const std::shared_ptr<SceneObject>& object);

	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};