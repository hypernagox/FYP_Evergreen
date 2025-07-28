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
		// General states
		Idle,
		Run,
		Attack,

		// Boss specific states
		BossTakeoff,
		BossLanding,
		BossFlyIdle,
		BossFlyRun,
		BossFlyAttack,
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
	float m_hitfactor = 0.0f;
	Vector3 m_lastPosition = Vector3::Zero;

public:
	Transform* m_transformBody;

	void OnInitialize() override;

	void InitializeMonster(std::string_view monsterType);
	void Update(const Time& time, Scene& scene) override;
	const auto& GetRenderObjTransform()const noexcept { return m_rendererObj->GetTransform(); }
	void OnAttackToPlayer();
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to);
	virtual void OnDeath() {}
	int GetHP() const { return m_hp; }
	virtual void OnHit(int afterHealth);
	virtual int GetMarkIndex() const { return 0; } // Default implementation, can be overridden by derived classes
};

class MonsterRemains : public Component
{
public:
	void OnInitialize() override;
	void OnActive() override;
	void InitializeMonster(Transform* bodyTransform, RiggedMeshRenderer* renderer, const Animation* deathAnimation, bool boss = false);
	void Update(const Time& time, Scene& scene) override;

private:
	std::shared_ptr<SceneObject> m_rendererObj;
	RiggedMeshRenderer* m_renderer = nullptr;
	static std::unique_ptr<SoundEffectInstance> m_soundInstance;
	std::unique_ptr<SoundEffectInstance> m_additionalSoundInstance;
	float m_lifeTime = 1.0f;
	bool m_isBoss = false;
};