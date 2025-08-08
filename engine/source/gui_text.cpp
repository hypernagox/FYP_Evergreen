#include "pch.h"
#include "gui_text.h"
#include "scene.h"
#include "font.h"

namespace udsdx
{
	void GUIText::Render(RenderParam& param)
	{
		const float height = RefScreenSize.y;
		const float width = height * param.AspectRatio;
		float ratio = param.Viewport.Height / height;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		Vector2 size = m_font->GetSpriteFont()->MeasureString(m_text.c_str());
		Quaternion rotation = GetTransform()->GetWorldRotation();
		Vector2 origin;
		switch (m_alignment)
		{
		case Alignment::UpperLeft:
			origin = Vector2(0.0f, 0.0f);
			break;
		case Alignment::UpperCenter:
			origin = Vector2(0.5f, 0.0f);
			break;
		case Alignment::UpperRight:
			origin = Vector2(1.0f, 0.0f);
			break;
		case Alignment::Left:
			origin = Vector2(0.0f, 0.5f);
			break;
		case Alignment::Center:
			origin = Vector2(0.5f, 0.5f);
			break;
		case Alignment::Right:
			origin = Vector2(1.0f, 0.5f);
			break;
		case Alignment::LowerLeft:
			origin = Vector2(0.0f, 1.0f);
			break;
		case Alignment::LowerCenter:
			origin = Vector2(0.5f, 1.0f);
			break;
		case Alignment::LowerRight:
			origin = Vector2(1.0f, 1.0f);
			break;
		}
		Vector3 scale = GetTransform()->GetLocalScale();
		m_font->GetSpriteFont()->DrawString(param.SpriteBatchNonPremultipliedAlpha, m_text.c_str(), position, m_color, -rotation.ToEuler().z, size * origin, scale * ratio);
	}
}