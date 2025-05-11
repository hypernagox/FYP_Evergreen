#include "pch.h"
#include "InteractiveEntity.h"
#include "GameScene.h"

using namespace udsdx;

InteractiveEntity::InteractiveEntity(const std::shared_ptr<SceneObject>& owner) : Component(owner)
{
	m_targetRenderer = owner->GetComponent<RendererBase>();
	if (m_targetRenderer == nullptr)
	{
		const auto& renderers = owner->GetComponentsInChildren<RendererBase>();
		if (renderers.size() > 0)
		{
			m_targetRenderer = renderers[0];
		}
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
	if (m_targetRenderer != nullptr)
	{
		m_targetRenderer->SetDrawOutline(inRange);
	}
}