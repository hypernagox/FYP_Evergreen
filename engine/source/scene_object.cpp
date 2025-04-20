#include "pch.h"
#include "scene_object.h"
#include "transform.h"
#include "component.h"
#include "core.h"

namespace udsdx
{
	void SceneObject::Enumerate(const std::shared_ptr<SceneObject>& root, std::function<void(const std::shared_ptr<SceneObject>&)> callback)
	{
		static std::stack<std::shared_ptr<SceneObject>> s;
		std::shared_ptr<SceneObject> node = root;

		// Perform in-order traversal (sibiling-node-child)
		// Since the order of the siblings is reversed, it needs to visit the siblings first
		while (!s.empty() || node != nullptr)
		{
			if (node != nullptr)
			{
				s.emplace(node);
				node = node->m_sibling;
			}
			else
			{
				// node is guaranteed to have an instance
				node = s.top();
				s.pop();
				if (node->m_active)
				{
					callback(node);
					node = node->m_child;
				}
				else
				{
					node = nullptr;
				}
			}
		}
	}

	void SceneObject::EnumerateUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene)
	{
		Enumerate(root, [&](const std::shared_ptr<SceneObject>& node) { node->Update(time, scene); });
	}

	void SceneObject::EnumeratePostUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene)
	{
		static std::stack<std::pair<std::shared_ptr<SceneObject>, bool>> s;
		std::pair<std::shared_ptr<SceneObject>, bool> node = std::make_pair(root, false);

		// Perform in-order traversal (sibiling-node-child)
		// Since the order of the siblings is reversed, it needs to visit the siblings first
		while (!s.empty() || node.first != nullptr)
		{
			if (node.first != nullptr)
			{
				s.emplace(node);
				node = std::make_pair(node.first->m_sibling, node.second);
			}
			else
			{
				node = s.top();
				s.pop();

				// SceneObject::PostUpdate() returns true if the node is still valid and active
				if (node.first->PostUpdate(time, scene, node.second))
				{
					node = std::make_pair(node.first->m_child, node.second);
				}
				else
				{
					node = std::make_pair(nullptr, node.second);
				}
			}
		}
	}

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

	void SceneObject::DetachFromHierarchy()
	{
		INSTANCE(Core)->FlushCommandQueue();

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

		m_detachDirty = false;
	}

	void SceneObject::Update(const Time& time, Scene& scene)
	{
		// Update components
		for (auto& component : m_components)
		{
			component->Update(time, scene);
		}
	}

	bool SceneObject::PostUpdate(const Time& time, Scene& scene, bool& forceValidate)
	{
		if (m_detachDirty)
		{
			DetachFromHierarchy();
			return false;
		}
		if (!m_active)
		{
			return false;
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
			component->PostUpdate(time, scene);
		}

		return true;
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
		m_detachDirty = true;
	}

	const SceneObject* SceneObject::GetParent() const
	{
		return m_parent;
	}

	bool SceneObject::GetActive() const
	{
		return m_active;
	}

	bool SceneObject::GetActiveInHierarchy() const
	{
		const SceneObject* node = this;
		while (node != nullptr)
		{
			if (!node->m_active)
			{
				return false;
			}
			node = node->m_parent;
		}
		return true;
	}

	void SceneObject::SetActive(bool active)
	{
		m_active = active;
	}
}