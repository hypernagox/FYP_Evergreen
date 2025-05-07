#pragma once

#include "pch.h"

using namespace udsdx;

class PlayerRenderer : public Component
{
public:
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
		Hit,
		Death,
		Size,
	};
protected:
	RiggedMeshRenderer* m_renderer;
	std::shared_ptr<SceneObject> m_rendererObj;
	std::array<std::shared_ptr<udsdx::Material>, 5> m_playerMaterials;
	std::shared_ptr<udsdx::Material> m_toolMaterial;
	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	
public:
	int m_attackState = 0;
	Transform* m_transformBody;
	PlayerRenderer(const std::shared_ptr<SceneObject>& object);
	~PlayerRenderer();

	void Update(const Time& time, Scene& scene) override;

	Transform* const GetRenderObjTransform() const noexcept { return m_rendererObj->GetTransform(); }
	void SetRotation(const Quaternion& rotation) { m_rendererObj->GetTransform()->SetLocalRotation(rotation); }
	void SetAnimation(AnimationClip* animationClip) { m_renderer->SetAnimation(animationClip); }
	void OnAnimationStateChange(const AnimationState& state);
	void Attack() { *m_stateMachine->GetConditionRefBool("Attack") = true; }
	void Hit() { *m_stateMachine->GetConditionRefBool("Hit") = true; }
	void Death() { *m_stateMachine->GetConditionRefBool("Death") = true; }
	bool TrySetState(AnimationState state) { return m_stateMachine->TrySetState(state); }
	AnimationState GetCurrentState() const { return m_stateMachine->GetCurrentState(); }
	bool GetIsRunning() const;
};