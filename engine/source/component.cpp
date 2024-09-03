#include "pch.h"
#include "component.h"
#include "scene_object.h"

namespace udsdx
{
	Component::Component(const std::shared_ptr<SceneObject>& object)
	{
		m_object = object;
	}

	Component::~Component()
	{

	}

	std::shared_ptr<SceneObject> Component::GetSceneObject() const
	{
		return m_object.lock();
	}

	Transform* Component::GetTransform()
	{
		return GetSceneObject()->GetTransform();
	}
}