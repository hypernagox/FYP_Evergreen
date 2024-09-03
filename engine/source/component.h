#pragma once

#include "pch.h"

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
		virtual void Update(const Time& time, Scene& scene) = 0;

	public:
		std::shared_ptr<SceneObject> GetSceneObject() const;
		Transform* GetTransform();
		template <typename Component_T>
		Component_T* AddComponent() { return GetSceneObject()->AddComponent<Component_T>(); }
		template <typename Component_T>
		Component* GetComponent() const { return GetSceneObject()->GetComponent<Component_T>(); }

	protected:
		std::weak_ptr<SceneObject> m_object;
	};
}