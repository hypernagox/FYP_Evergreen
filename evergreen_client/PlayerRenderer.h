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
		Hit,
		Death,
		Size,
	};
protected:
	RiggedMeshRenderer* m_renderer;
	std::shared_ptr<SceneObject> m_rendererObj;
	std::shared_ptr<SceneObject> m_bodyObj;
	std::array<std::shared_ptr<udsdx::Material>, 5> m_playerMaterials;
	std::shared_ptr<udsdx::Material> m_toolMaterial;
	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	
public:
	CharacterType m_characterType = CharacterType::Warrior;
	int m_attackState = 0;
	Transform* m_transformBody;

	void OnInitialize() override;
	void Update(const Time& time, Scene& scene) override;

	void InitializeWarrior();
	void InitializePriest();

	Transform* const GetRenderObjTransform() const noexcept { return m_rendererObj->GetTransform(); }
	void SetEquipmentState(bool isEquipped);
	void SetRotation(const Quaternion& rotation) { m_rendererObj->GetTransform()->SetLocalRotation(rotation); }
	void SetAnimation(AnimationClip* animationClip, bool loop, bool forcePlay) { m_renderer->SetAnimation(animationClip, loop, forcePlay); }
	void OnAnimationStateChange(const AnimationState& state);
	void Attack() { *m_stateMachine->GetConditionRefBool("Attack") = true; }
	void Hit() { *m_stateMachine->GetConditionRefBool("Hit") = true; }
	void Death() { *m_stateMachine->GetConditionRefBool("Death") = true; }
	bool TrySetState(AnimationState state) { return m_stateMachine->TrySetState(state); }
	AnimationState GetCurrentState() const { return m_stateMachine->GetCurrentState(); }
	bool GetIsRunning() const;
	void SetPlayerWeapon(std::string_view weaponName);
};