#include "pch.h"
#include "EntityInteraction.h"
#include "InteractiveEntity.h"
#include "InteractionFloatGUI.h"
#include "InputHandler.h"
#include "GameScene.h"

using namespace udsdx;

EntityInteraction::EntityInteraction(const std::shared_ptr<SceneObject>& owner) : Component(owner)
{
	auto inputHandler = owner->GetComponent<InputHandler>();
	inputHandler->AddKeyFunc(Keyboard::E, KEY_STATE::KET_TAP, &EntityInteraction::Interact, this);
}

void EntityInteraction::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	InteractiveEntity* closestEntity = GetInteractiveEntity();
	if (closestEntity != nullptr)
	{
		m_interactionFloatGUI->GetSceneObject()->SetActive(true);
		m_interactionFloatGUI->SetTargetPosition(closestEntity->GetTransform()->GetLocalPosition());
		m_interactionFloatGUI->SetText(closestEntity->GetInteractionText());
	}
	else
	{
		m_interactionFloatGUI->GetSceneObject()->SetActive(false);
	}
}

void EntityInteraction::Interact()
{
	InteractiveEntity* closestEntity = GetInteractiveEntity();
	if (closestEntity != nullptr)
	{
		closestEntity->OnInteract();
	}
}

InteractiveEntity* EntityInteraction::GetInteractiveEntity()
{
	std::vector<InteractiveEntity*> interactiveEntities = m_targetScene->GetInteractiveEntities();
	InteractiveEntity* closestEntity = nullptr;
	float closestDistanceSqr = std::numeric_limits<float>::max();
	for (InteractiveEntity* entity : interactiveEntities)
	{
		if (!entity->GetActive())
			continue;
		float distanceSqr = (GetTransform()->GetLocalPosition() - entity->GetTransform()->GetLocalPosition()).LengthSquared();
		if (distanceSqr < closestDistanceSqr)
		{
			closestDistanceSqr = distanceSqr;
			closestEntity = entity;
		}
	}
	if (closestEntity != nullptr && closestDistanceSqr <= m_interactionRange * m_interactionRange)
		return closestEntity;
	return nullptr;
}
