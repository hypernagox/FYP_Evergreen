#pragma once

#include "pch.h"
#include "transform.h"

namespace udsdx
{
	class Component;
	class Scene;
	class Camera;

	class SceneObject : public std::enable_shared_from_this<SceneObject>
	{
		friend class Scene;

	public:
		class GarbageCollector
		{
		public:
			static void Stash(SceneObject* object);
			static void Collect(int frameResourceIndex);

		private:
			static int m_currentFrameResourceIndex;
			static std::array<std::vector<SceneObject*>, FrameResourceCount + 1> m_garbage;
		};

	private:
		struct SceneObjectDeleter
		{
			void operator()(SceneObject* object) const;
		};

		struct ComponentDeleter
		{
			void operator()(Component* component) const;
		};
		
	public:
		static void Enumerate(const std::shared_ptr<SceneObject>& root, std::function<void(const std::shared_ptr<SceneObject>&)> callback, bool onlyActive = true);
		static std::shared_ptr<SceneObject> MakeShared();

	private:
		SceneObject();
		SceneObject(const SceneObject& rhs) = delete;
		SceneObject& operator=(const SceneObject& rhs) = delete;
		~SceneObject();

	public:
		Transform* GetTransform();
		void Update(const Time& time, Scene& scene);
		void PostUpdate(const Time& time, Scene& scene);
		void OnDrawGizmos(const Camera* target);

	public:
		void AddChild(std::shared_ptr<SceneObject> child);
		void RemoveFromParent();
		const SceneObject* GetParent() const;

	public:
		bool GetActive() const;
		bool GetActiveInHierarchy() const;
		bool GetActiveInScene() const;
		Scene* GetScene() const;
		bool GetAttachedInScene() const;
		void SetActive(bool active);

	public:
		template <typename Component_T>
		Component_T* AddComponent()
		{
			std::unique_ptr<Component_T, ComponentDeleter> component = std::unique_ptr<Component_T, ComponentDeleter>(new Component_T());
			Component_T* componentPtr = component.get();
			componentPtr->m_object = shared_from_this();
			m_components.emplace_back(std::move(component));
			componentPtr->OnInitialize();
			bool attachedInScene = GetAttachedInScene();
			bool activeInScene = GetActiveInScene();
			if (attachedInScene)
			{
				componentPtr->OnAttach();
			}
			if (activeInScene)
			{
				componentPtr->OnActive();
			}
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
			Enumerate(std::const_pointer_cast<SceneObject>(shared_from_this()), [&](const std::shared_ptr<SceneObject>& node) {
				if (Component_T* component = node->GetComponent<Component_T>())
				{
					components.emplace_back(component);
				}
			});
			return components;
		}

		template <typename Component_T>
		Component_T* GetComponentInParent() const
		{
			const SceneObject* parent = GetParent();
			while (nullptr != parent)
			{
				if (Component_T* component = parent->GetComponent<Component_T>())
				{
					return component;
				}
				parent = parent->GetParent();
			}
			return nullptr;
		}

		template <typename Component_T>
		std::vector<Component_T*> GetComponentsInParent() const
		{
			std::vector<Component_T*> components;
			const SceneObject* parent = GetParent();
			while (nullptr != parent)
			{
				if (Component_T* component = parent->GetComponent<Component_T>())
				{
					components.push_back(component);
				}
				parent = parent->GetParent();
			}
			return components;
		}

		void RemoveAllComponents();

	protected:
		bool m_active = true;
		// Only has an instance for the root SceneObject of a Scene
		Scene* m_sceneRoot = nullptr;
		Transform m_transform = Transform();
		std::vector<std::unique_ptr<Component, ComponentDeleter>> m_components;

	protected:
		SceneObject* m_parent = nullptr;
		SceneObject* m_back = nullptr;
		std::shared_ptr<SceneObject> m_sibling = nullptr;
		std::shared_ptr<SceneObject> m_child = nullptr;
	};
}