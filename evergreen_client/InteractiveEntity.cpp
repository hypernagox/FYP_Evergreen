#include "pch.h"
#include "InteractiveEntity.h"
#include "GameScene.h"

using namespace udsdx;

InteractiveEntity::InteractiveEntity(const std::shared_ptr<SceneObject>& owner) : Component(owner)
{
	m_targetRenderers = owner->GetComponentsInChildren<RendererBase>();
	RendererBase* renderer = owner->GetComponent<RendererBase>();
	if (nullptr != renderer)
	{
		m_targetRenderers.emplace_back(renderer);
	}
}

void InteractiveEntity::OnInteract()
{
	if (m_interactionCallback)
	{
		m_interactionCallback();
	}
	else
	{
		DebugConsole::Log(L"Interaction callback not set.");
	}
}

void InteractiveEntity::OnInteractRange(bool inRange)
{
	for (const auto& renderer : m_targetRenderers)
	{
		renderer->SetDrawOutline(inRange);
	}
}