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
		SceneObject();
		SceneObject(const SceneObject& rhs) = delete;
		SceneObject& operator=(const SceneObject& rhs) = delete;
		~SceneObject();

	public:
		Transform* GetTransform();
		void Update(const Time& time, Scene& scene, bool forceValidate);

	public:
		void AddChild(std::shared_ptr<SceneObject> child);
		void RemoveFromParent();
		std::shared_ptr<SceneObject> GetParent() const;

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
		void RemoveAllComponents();

	protected:
		Transform m_transform = Transform();
		std::vector<std::unique_ptr<Component>> m_components;

	protected:
		SceneObject* m_parent = nullptr;
		std::shared_ptr<SceneObject> m_sibling = nullptr;
		std::shared_ptr<SceneObject> m_child = nullptr;
	};
}