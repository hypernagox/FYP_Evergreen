#include "pch.h"
#include "ServerComponent.h"
#include "ServerObject.h"

ServerComponent::ServerComponent(ServerObject* const owner)
	: m_owner{ owner }
{
}

ServerComponent::~ServerComponent()
{
}

SceneObject* const ServerComponent::GetRootObject() const noexcept
{
	return m_owner->GetSceneObject().get();
}
