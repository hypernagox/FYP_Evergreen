#include "pch.h"
#include "component.h"
#include "scene_object.h"

namespace udsdx
{
	std::unordered_map<std::string, size_t> Component::s_componentCounts;
	std::mutex Component::s_componentCountsMutex;

	const std::unordered_map<std::string, size_t>& Component::GetComponentCounts()
	{
		return s_componentCounts;
	}

	Component::Component()
	{
	}

	Component::~Component()
	{
	}

	void Component::RegisterComponentInstance()
	{
		std::lock_guard<std::mutex> lock(s_componentCountsMutex);
		const char* typeName = typeid(*this).name();
		s_componentCounts[typeName]++;
	}

	void Component::UnregisterComponentInstance()
	{
		std::lock_guard<std::mutex> lock(s_componentCountsMutex);
		const char* typeName = typeid(*this).name();
		auto it = s_componentCounts.find(typeName);
		if (it != s_componentCounts.end() && it->second > 0)
		{
			it->second--;
			if (it->second == 0)
			{
				s_componentCounts.erase(it);
			}
		}
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