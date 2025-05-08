#include "pch.h"
#include "InteractiveEntity.h"
#include "GameScene.h"

using namespace udsdx;

InteractiveEntity::InteractiveEntity(const std::shared_ptr<SceneObject>& owner) : Component(owner)
{
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
