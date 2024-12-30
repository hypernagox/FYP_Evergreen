#pragma once

#include "pch.h"

using namespace udsdx;

class EntityMovement;

class Monster : public Component
{
	enum class AnimationState
	{
		Idle,
		Attack,
		Size
	};

protected:
	std::shared_ptr<SceneObject> m_rendererObj;
	EntityMovement* m_entityMovement;

	std::shared_ptr<udsdx::Material> m_monsterMaterial;
	udsdx::RiggedMeshRenderer* m_riggedMeshRenderer;

	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;

public:
	Transform* m_transformBody;
	Monster(const std::shared_ptr<SceneObject>& object);
	~Monster();

	void Update(const Time& time, Scene& scene) override;
	const auto& GetRenderObjTransform()const noexcept { return m_rendererObj->GetTransform(); }
	void OnAttackToPlayer();
	void OnAnimationStateChange(AnimationState from, AnimationState to);
};