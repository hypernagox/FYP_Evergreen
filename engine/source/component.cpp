#include "pch.h"
#include "component.h"
#include "scene_object.h"

namespace udsdx
{
	Component::Component()
	{
	}

	Component::~Component()
	{
	}

	void Component::OnInitialize()
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

	void Component::SetActive(bool active)
	{
		if (m_isActive == active)
		{
			return;
		}

		m_isActive = active;
		if (GetSceneObject()->GetActiveInScene())
		{
			if (active)
			{
				OnActive();
			}
			else
			{
				OnInactive();
			}
		}
	}

	Transform* Component::GetTransform()
	{
		return GetSceneObject()->GetTransform();
	}
}