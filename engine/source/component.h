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
		friend class SceneObject;

	protected:
		Component();
		virtual ~Component();

	public:
		// Called when the component is created and attached to a SceneObject
		virtual void OnInitialize();

		// Called when the component is attached to a active scene
		virtual void OnAttach();

		// Called when:
		// - After OnAttach() if the component is hierarchically active
		// - The component is hierarchically activated by SceneObject::SetActive(true) or Component::SetActive(true)
		virtual void OnActive();

		// Called when the component is about to call Update() function for the first time
		virtual void Begin();

		friend class SceneObject;
		// Called every frame
		virtual void Update(const Time& time, Scene& scene);

		// Called every frame after Update()
		// In this function:
		// - Do not detach any SceneObject in this function; this may cause dangling pointers
		// - Modifying another Transform is not recommended; children of the other Transform may not have the latest global matrix
		virtual void PostUpdate(const Time& time, Scene& scene);

		// Called when the component needs to draw ImGUI primitives
		virtual void OnDrawGizmos(const Camera* target);

		// Called when:
		// - Before OnDetach() if the component is hierachically active
		// - The component is hierarchically deactivated by SceneObject::SetActive(false) or Component::SetActive(false)
		virtual void OnInactive();

		// Called when the component is detached from the scene
		virtual void OnDetach();

	public:
		std::shared_ptr<SceneObject> GetSceneObject() const;
		bool GetActive() const { return m_isActive; }
		void SetActive(bool active);
		Transform* GetTransform();
		template <typename Component_T>
		Component_T* AddComponent() { return GetSceneObject()->AddComponent<Component_T>(); }
		template <typename Component_T>
		Component_T* GetComponent() const { return GetSceneObject()->GetComponent<Component_T>(); }

	protected:
		std::weak_ptr<SceneObject> m_object;
		bool m_isActive = true;
		bool m_isBegin = true;
	};
}