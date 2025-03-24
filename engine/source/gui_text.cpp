#include "pch.h"
#include "gui_text.h"
#include "scene.h"
#include "font.h"

namespace udsdx
{
	GUIText::GUIText(const std::shared_ptr<SceneObject>& object) : GUIElement(object)
	{
	}

	void GUIText::Render(RenderParam& param)
	{
		m_font->GetSpriteFont()->DrawString(param.SpriteBatchNonPremultipliedAlpha, m_text.c_str(), Vector2(100.0f, 100.0f));
	}
}