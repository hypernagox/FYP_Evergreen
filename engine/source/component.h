#pragma once

#include "pch.h"
#include "scene_object.h"

namespace udsdx
{
	class SceneObject;
	class Transform;
	class Scene;

	class Component
	{
	public:
		Component() = delete;
		Component(const std::shared_ptr<SceneObject>& object);
		virtual ~Component();

	public:
		virtual void Update(const Time& time, Scene& scene);
		virtual void PostUpdate(const Time& time, Scene& scene);

	public:
		std::shared_ptr<SceneObject> GetSceneObject() const;
		bool GetActive() const { return m_isActive; }
		void SetActive(bool active) { m_isActive = active; }
		Transform* GetTransform();
		template <typename Component_T>
		Component_T* AddComponent() { return GetSceneObject()->AddComponent<Component_T>(); }
		template <typename Component_T>
		Component_T* GetComponent() const { return GetSceneObject()->GetComponent<Component_T>(); }

	protected:
		std::weak_ptr<SceneObject> m_object;
		bool m_isActive = true;
	};
}