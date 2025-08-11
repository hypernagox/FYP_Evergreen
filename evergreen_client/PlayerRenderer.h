#pragma once

#include "pch.h"

using namespace udsdx;

class PlayerRenderer : public Component
{
public:
	enum class CharacterType
	{
		Warrior,
		Priest,
		Archer,
	};
	enum class AnimationState
	{
		Idle,
		RunIntermediate,
		RunForward,
		RunBackward,
		RunLeft,
		RunRight,
		RunLeftForward,
		RunLeftBackward,
		RunRightForward,
		RunRightBackward,
		Attack,
		AttackEnd,
		Dash,
		Hit,
		HitEnd,
		Death,
		Size,
	};
protected:
	RiggedMeshRenderer* m_renderer;
	std::shared_ptr<SceneObject> m_rendererObj;
	std::shared_ptr<SceneObject> m_bodyObj;
	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	std::unique_ptr<SoundEffectInstance> footStepSFXInstance;

	float m_viewYaw = 0.0f;
	float m_viewPitch = 0.0f;
	float m_viewYawTarget = 0.0f;
	float m_viewPitchTarget = 0.0f;
	
public:
	CharacterType m_characterType = CharacterType::Warrior;
	int m_attackState = 0;
	float m_particleTimer = 0.0f;
	Transform* m_transformBody;

	void OnInitialize() override;
	void Update(const Time& time, Scene& scene) override;

	void InitializeWarrior();
	void InitializePriest();
	void InitializeArcher();

	Transform* const GetRenderObjTransform() const noexcept { return m_rendererObj->GetTransform(); }
	void SetRotation(const Quaternion& rotation) { m_rendererObj->GetTransform()->SetLocalRotation(rotation); }
	void SetAnimation(AnimationClip* animationClip, bool loop, bool forcePlay) { m_renderer->SetAnimation(animationClip, loop, forcePlay); }
	void SetViewDirection(float yaw, float pitch);
	void UpdateViewDirection(float deltaTime);
	void OnAnimationStateChange(const AnimationState& state);
	void Attack() { *m_stateMachine->GetConditionRefBool("Attack") = true; }
	void Dash() { *m_stateMachine->GetConditionRefBool("Dash") = true; }
	void Hit() { *m_stateMachine->GetConditionRefBool("Hit") = true; }
	void Death() { *m_stateMachine->GetConditionRefBool("Death") = true; }
	bool TrySetState(AnimationState state) { return m_stateMachine->TrySetState(state); }
	AnimationState GetCurrentState() const { return m_stateMachine->GetCurrentState(); }
	CharacterType GetCharacterType() const { return m_characterType; }
	bool GetIsRunning() const;

	void SetPlayerWeapon(int weaponID);
	void SetPlayerArmor(int armorID);
};