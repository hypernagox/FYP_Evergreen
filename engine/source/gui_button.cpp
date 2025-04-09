#include "pch.h"
#include "gui_button.h"
#include "texture.h"

namespace udsdx
{
	GUIButton::GUIButton(const std::shared_ptr<SceneObject>& object) : GUIImage(object)
	{
	}

	void GUIButton::Render(RenderParam& param)
	{
		float ratio = param.Viewport.Height / RefScreenSize.y;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		Vector4 color = Colors::White;
		if (GetMouseHovering())
		{
			color = Colors::LightGray;
		}
		if (GetMousePressing())
		{
			color = Colors::DimGray;
		}
		if (m_texture != nullptr)
		{
			param.SpriteBatchNonPremultipliedAlpha->Draw(
				m_texture->GetSrvGpu(),
				XMUINT2(static_cast<UINT>(m_size.x), static_cast<UINT>(m_size.y)),
				position,
				nullptr,
				color,
				0.0f,
				Vector2(m_size.x, m_size.y) * 0.5f,
				Vector2::One * ratio
			);
		}
	}

	void GUIButton::OnMouseRelease()
	{
		if (m_clickCallback)
		{
			m_clickCallback();
		}
	}
}
