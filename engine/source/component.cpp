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

	void Component::OnAttach()
	{
	}

	void Component::OnActive()
	{
	}

	void Component::Begin()
	{
	}

	void Component::Update(const Time& time, Scene& scene)
	{
	}

	void Component::PostUpdate(const Time& time, Scene& scene)
	{
	}

	void Component::OnDrawGizmos(const Camera* target)
	{
	}

	void Component::OnInactive()
	{
	}

	void Component::OnDetach()
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