#pragma once

#include "pch.h"

using namespace udsdx;

class EntityMovement;
class MonsterHPPanel;

class Monster : public Component
{
protected:
	enum class AnimationState
	{
		Idle,
		Run,
		Attack,
		Size
	};

protected:
	RiggedMeshRenderer* m_renderer = nullptr;
	std::shared_ptr<SceneObject> m_rendererObj;
	EntityMovement* m_entityMovement;

	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	MonsterHPPanel* m_hpPanel;

	int m_hp = 10;
	int m_maxHP = 10;
	Vector3 m_lastPosition = Vector3::Zero;

public:
	Transform* m_transformBody;

	void OnInitialize() override;

	void InitializeMonster(std::string_view monsterType);
	void Update(const Time& time, Scene& scene) override;
	const auto& GetRenderObjTransform()const noexcept { return m_rendererObj->GetTransform(); }
	void OnAttackToPlayer();
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to);
	int GetHP() const { return m_hp; }
	void OnHit(int afterHealth);
};