#include "pch.h"
#include "gui_element.h"
#include "scene.h"
#include "core.h"
#include "input.h"

namespace udsdx
{
	GUIElement::GUIElement(const std::shared_ptr<SceneObject>& object) : Component(object)
	{
	}

	void GUIElement::UpdateEvent(bool hover)
	{
		if (m_mouseHovering != hover)
		{
			m_mouseHovering = hover;
			if (m_mouseHovering)
			{
				OnMouseEnter();
			}
			else
			{
				m_mousePressing = false;
				OnMouseExit();
			}
		}

		if (m_mouseHovering)
		{
			OnMouseHover();
			if (INSTANCE(Input)->GetMouseLeftButtonDown())
			{
				m_mousePressing = true;
				OnMousePress();
			}
			if (m_mousePressing && INSTANCE(Input)->GetMouseLeftButtonUp())
			{
				m_mousePressing = false;
				OnMouseRelease();
			}
		}
	}

	void GUIElement::PostUpdate(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderGUIObject(this);
	}

	RECT GUIElement::GetScreenRect() const
	{
		Vector2 ScreenSize = Vector2(static_cast<float>(INSTANCE(Core)->GetClientWidth()), static_cast<float>(INSTANCE(Core)->GetClientHeight()));
		float ratio = ScreenSize.y / RefScreenSize.y;

		Vector2 position = GetSceneObject()->GetTransform()->GetWorldPosition();
		Vector2 screenPosition = position * Vector2(ratio, -ratio) + ScreenSize / 2;
		Vector2 screenHalfSize = m_size * ratio / 2;

		return RECT{
			static_cast<LONG>(screenPosition.x - screenHalfSize.x),
			static_cast<LONG>(screenPosition.y - screenHalfSize.y),
			static_cast<LONG>(screenPosition.x + screenHalfSize.x),
			static_cast<LONG>(screenPosition.y + screenHalfSize.y)
		};
	}
}