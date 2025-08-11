#pragma once

#include "pch.h"

class EntityMovement;

class NPCRenderer : public udsdx::Component
{
protected:
	enum class AnimationState
	{
		// General states
		Idle,
		Run,
		Chat,
		Size
	};

public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void OnAnimationStateChange(AnimationState from, AnimationState to);
	void ChangeChatState(bool state);

private:
	std::shared_ptr<udsdx::SceneObject> m_rendererObject;
	std::unique_ptr<Common::StateMachine<AnimationState>> m_stateMachine;
	Vector3 m_lastPosition = Vector3::Zero;
	EntityMovement* m_entityMovement;
};

