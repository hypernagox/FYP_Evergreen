#pragma once

#include "pch.h"

using namespace udsdx;

class PlayerRenderer : public Component
{
public:
	enum class AnimationState
	{
		Idle,
		Run,
		Attack,
		Hit,
		Death,
		Size,
	};
protected:
	RiggedMeshRenderer* m_renderer;
	std::shared_ptr<SceneObject> m_rendererObj;
	std::array<std::shared_ptr<udsdx::Material>, 5> m_playerMaterials;
	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	
public:
	Transform* m_transformBody;
	PlayerRenderer(const std::shared_ptr<SceneObject>& object);
	~PlayerRenderer();

	void Update(const Time& time, Scene& scene) override;

	Transform* const GetRenderObjTransform() const noexcept { return m_rendererObj->GetTransform(); }
	void SetRotation(const Quaternion& rotation) { m_rendererObj->GetTransform()->SetLocalRotation(rotation); }
	void SetAnimation(const std::string& animationName) { m_renderer->SetAnimation(animationName); }
	void OnAnimationStateChange(const AnimationState& state);
	void Attack() { *m_stateMachine->GetConditionRefBool("Attack") = true; }
	void Hit() { *m_stateMachine->GetConditionRefBool("Hit") = true; }
	void Death() { *m_stateMachine->GetConditionRefBool("Death") = true; }
	bool TrySetState(AnimationState state) { return m_stateMachine->TrySetState(state); }
};