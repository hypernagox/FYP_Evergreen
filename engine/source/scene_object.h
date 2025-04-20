#pragma once

#include "pch.h"
#include "transform.h"

namespace udsdx
{
	class Component;
	class Scene;

	class SceneObject : public std::enable_shared_from_this<SceneObject>
	{
	public:
		static void Enumerate(const std::shared_ptr<SceneObject>& root, std::function<void(const std::shared_ptr<SceneObject>&)> callback);
		static void EnumerateUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene);
		static void EnumeratePostUpdate(const std::shared_ptr<SceneObject>& root, const Time& time, Scene& scene);

	public:
		SceneObject();
		SceneObject(const SceneObject& rhs) = delete;
		SceneObject& operator=(const SceneObject& rhs) = delete;
		~SceneObject();

	public:
		Transform* GetTransform();
		void Update(const Time& time, Scene& scene);
		bool PostUpdate(const Time& time, Scene& scene, bool& forceValidate);

	public:
		void AddChild(std::shared_ptr<SceneObject> child);
		void RemoveFromParent();
		const SceneObject* GetParent() const;

	public:
		bool GetActive() const;
		bool GetActiveInHierarchy() const;
		void SetActive(bool active);

	public:
		template <typename Component_T>
		Component_T* AddComponent()
		{
			std::unique_ptr<Component_T> component = std::make_unique<Component_T>(shared_from_this());
			Component_T* componentPtr = component.get();
			m_components.emplace_back(std::move(component));
			return componentPtr;
		}

		template <typename Component_T>
		Component_T* GetComponent() const
		{
			for (auto& component : m_components)
			{
				Component_T* castedComponent = dynamic_cast<Component_T*>(component.get());
				if (castedComponent)
				{
					return castedComponent;
				}
			}
			return nullptr;
		}

		template <typename Component_T>
		std::vector<Component_T*> GetComponentsInChildren() const
		{
			std::vector<Component_T*> components;
			Enumerate(m_child, [&](const std::shared_ptr<SceneObject>& node) {
				if (Component_T* component = node->GetComponent<Component_T>())
				{
					components.push_back(component);
				}
			});
			return components;
		}

		void RemoveAllComponents();

	private:
		void DetachFromHierarchy();

	protected:
		bool m_active = true;
		Transform m_transform = Transform();
		std::vector<std::unique_ptr<Component>> m_components;

	protected:
		bool m_detachDirty = false;
		SceneObject* m_parent = nullptr;
		std::shared_ptr<SceneObject> m_sibling = nullptr;
		std::shared_ptr<SceneObject> m_child = nullptr;
	};
}