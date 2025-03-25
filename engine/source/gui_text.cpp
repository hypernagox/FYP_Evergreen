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
		const float height = 1080.0f;
		const float width = height * param.AspectRatio;
		float ratio = param.Viewport.Height / height;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		Vector2 size = m_font->GetSpriteFont()->MeasureString(m_text.c_str());
		m_font->GetSpriteFont()->DrawString(param.SpriteBatchNonPremultipliedAlpha, m_text.c_str(), position, Colors::White, 0.0f, size * 0.5f, Vector2::One * ratio);
	}
}