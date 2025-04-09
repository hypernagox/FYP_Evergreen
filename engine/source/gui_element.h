#pragma once

#include "component.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIElement : public Component
	{
	public:
		static constexpr Vector2 RefScreenSize = Vector2(1920, 1080);

	public:
		GUIElement(const std::shared_ptr<SceneObject>& object);

	public:
		void UpdateEvent(bool hover);
		void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param) = 0;

	protected:
		virtual void OnMouseEnter() { };
		virtual void OnMouseHover() { };
		virtual void OnMouseExit() { };
		virtual void OnMousePress() { };
		virtual void OnMouseRelease() { };

	public:
		Vector2 GetSize() const { return m_size; }
		RECT GetScreenRect() const;
		bool GetRaycastTarget() const { return m_raycastTarget; }

		bool GetMouseHovering() const { return m_mouseHovering; }
		bool GetMousePressing() const { return m_mousePressing; }

	public:
		void SetSize(const Vector2& value) { m_size = value; }
		void SetRaycastTarget(bool value) { m_raycastTarget = value; }

	protected:
		Vector2 m_size = Vector2(100, 100);
		bool m_raycastTarget = true;

		bool m_mouseHovering = false;
		bool m_mousePressing = false;
	};
}