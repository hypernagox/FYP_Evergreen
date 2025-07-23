#pragma once

#include "component.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIElement : public Component
	{
	public:
		static constexpr Vector2 RefScreenSize = Vector2(2560, 1440);

	public:
		void UpdateEvent(bool hover);
		void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param) = 0;

	protected:
		virtual void OnMouseEnter() { };
		virtual void OnMouseHover() { };
		virtual void OnMouseExit() { };
		virtual void OnMousePress(int mouse) { };
		virtual void OnMouseRelease(int mouse) { };

	public:
		Vector2 GetSize() const { return m_size; }
		Color GetColor() const { return m_color; }
		RECT GetScreenRect() const;
		bool GetRaycastTarget() const { return m_raycastTarget; }

		bool GetMouseHovering() const { return m_mouseHovering; }
		bool GetMousePressing() const { return m_leftMousePressing || m_rightMousePressing || m_middleMousePressing; }
		bool GetLeftMousePressing() const { return m_leftMousePressing; }
		bool GetRightMousePressing() const { return m_rightMousePressing; }
		bool GetMiddleMousePressing() const { return m_middleMousePressing; }

	public:
		void SetSize(const Vector2& value) { m_size = value; }
		void SetColor(const Color& value) { m_color = value; }
		void SetRaycastTarget(bool value) { m_raycastTarget = value; }

	protected:
		Vector2 m_size = Vector2(100, 100);
		Color m_color = Colors::White;
		bool m_raycastTarget = true;

		bool m_mouseHovering = false;
		bool m_leftMousePressing = false;
		bool m_rightMousePressing = false;
		bool m_middleMousePressing = false;
	};
}