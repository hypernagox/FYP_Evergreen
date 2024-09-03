#include "pch.h"
#include "scene_object.h"
#include "transform.h"
#include "component.h"

namespace udsdx
{
	SceneObject::SceneObject()
	{

	}

	SceneObject::~SceneObject()
	{

	}

	Transform* SceneObject::GetTransform()
	{
		return &m_transform;
	}

	void SceneObject::RemoveAllComponents()
	{
		m_components.clear();
	}

	void SceneObject::Update(const Time& time, Scene& scene, bool forceValidate)
	{
		// Update siblings backwards due to its order
		if (m_sibling != nullptr)
		{
			std::shared_ptr<SceneObject> sibling = m_sibling;
			sibling->Update(time, scene, forceValidate);
		}

		// Validate SRT matrix
		forceValidate |= m_transform.ValidateLocalSRTMatrix();
		if (forceValidate)
		{
			m_transform.ValidateWorldSRTMatrix();
		}

		// Update components
		for (auto& component : m_components)
		{
			component->Update(time, scene);
		}

		// Update children, recursively
		if (m_child != nullptr)
		{
			std::shared_ptr<SceneObject> child = m_child;
			m_child->Update(time, scene, forceValidate);
		}
	}

	void SceneObject::AddChild(std::shared_ptr<SceneObject> child)
	{
		if (m_child != nullptr)
		{
			m_child->m_parent = child.get();
		}

		child->m_sibling = m_child;
		child->m_parent = this;
		m_child = child;

		child->m_transform.SetParent(&m_transform);
	}

	void SceneObject::RemoveFromParent()
	{
		if (m_parent == nullptr)
		{
			return;
		}

		if (m_sibling != nullptr)
		{
			m_sibling->m_parent = m_parent;
		}
		if (m_parent->m_sibling.get() == this)
		{
			m_parent->m_sibling = m_sibling;
		}
		if (m_parent->m_child.get() == this)
		{
			m_parent->m_child = m_sibling;
		}

		m_parent = nullptr;
		m_sibling = nullptr;

		m_transform.SetParent(nullptr);
	}

	std::shared_ptr<SceneObject> SceneObject::GetParent() const
	{
		return m_parent->shared_from_this();
	}
}