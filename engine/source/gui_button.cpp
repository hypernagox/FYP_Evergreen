#include "pch.h"
#include "gui_button.h"
#include "texture.h"

namespace udsdx
{
	void GUIButton::Render(RenderParam& param)
	{
		float ratio = param.Viewport.Height / RefScreenSize.y;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		Quaternion rotation = GetTransform()->GetWorldRotation();
		Vector4 color = Colors::White;
		if (!m_interactable)
		{
			color = Colors::LightGray;
		}
		else
		{
			if (GetMouseHovering())
			{
				color = Colors::LightGray;
			}
			if (GetMousePressing())
			{
				color = Colors::DimGray;
			}
		}
		if (m_texture != nullptr)
		{
			Vector2Int textureSize = m_texture->GetSize();
			param.SpriteBatchNonPremultipliedAlpha->Draw(
				m_texture->GetSrvGpu(),
				XMUINT2(textureSize.x, textureSize.y),
				position,
				nullptr,
				color,
				-rotation.ToEuler().z,
				Vector2(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)) * 0.5f,
				Vector2(m_size.x / textureSize.x, m_size.y / textureSize.y) * ratio
			);
		}
	}

	void GUIButton::OnMouseRelease(int mouse)
	{
		if (m_interactable && m_clickCallback)
		{
			m_clickCallback();
		}
	}
}
