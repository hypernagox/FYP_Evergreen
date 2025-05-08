#pragma once

#include "pch.h"

class InteractionFloatGUI;
class InteractiveEntity;
class GameScene;

class EntityInteraction : public udsdx::Component
{
public:
	EntityInteraction(const std::shared_ptr<udsdx::SceneObject>& owner);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void Interact();
	void SetInteractionFloatGUI(InteractionFloatGUI* interactionFloatGUI) { m_interactionFloatGUI = interactionFloatGUI; }
	void SetTargetScene(GameScene* targetScene) { m_targetScene = targetScene; }
	InteractiveEntity* GetInteractiveEntity();

private:
	float m_interactionRange = 2.0f;

	InteractionFloatGUI* m_interactionFloatGUI = nullptr;
	GameScene* m_targetScene = nullptr;
};