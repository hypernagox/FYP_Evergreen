#include "pch.h"
#include "gui_image.h"
#include "scene.h"
#include "core.h"
#include "shader_compile.h"
#include "transform.h"
#include "texture.h"

namespace udsdx
{
	void GUIImage::Render(RenderParam& param)
	{
		if (m_texture != nullptr)
		{
			Matrix4x4 m = GetTransform()->GetWorldSRTMatrix();

			Vector3 xAxis(m._11, m._21, m._31);
			Vector3 yAxis(m._12, m._22, m._32);

			float scaleX = xAxis.Length();
			float scaleY = yAxis.Length();

			Vector2Int textureSize = m_texture->GetSize();
			RECT sourceRect = { 0, 0, textureSize.x, textureSize.y };
			Vector2 offset = Vector3::Zero;
			switch (m_fillType)
			{
			case udsdx::GUIImage::FillType::FillHorizontalRight:
				sourceRect.right = static_cast<LONG>(textureSize.x * m_fillAmount);
				break;
			case udsdx::GUIImage::FillType::FillHorizontalLeft:
				sourceRect.left = static_cast<LONG>(textureSize.x * (1.0f - m_fillAmount));
				offset.x = -static_cast<float>(textureSize.x) * (1.0f - m_fillAmount);
				break;
			case udsdx::GUIImage::FillType::FillVerticalUp:
				sourceRect.top = static_cast<LONG>(textureSize.y * (1.0f - m_fillAmount));
				offset.y = -static_cast<float>(textureSize.y) * (1.0f - m_fillAmount);
				break;
			case udsdx::GUIImage::FillType::FillVerticalDown:
				sourceRect.bottom = static_cast<LONG>(textureSize.y * m_fillAmount);
				break;
			}

			float ratio = param.Viewport.Height / RefScreenSize.y;
			Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
			Quaternion rotation = GetTransform()->GetWorldRotation();

			param.SpriteBatchNonPremultipliedAlpha->Draw(
				m_texture->GetSrvGpu(),
				XMUINT2(textureSize.x, textureSize.y),
				position,
				&sourceRect,
				m_color,
				-rotation.ToEuler().z,
				Vector2(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)) * 0.5f + offset,
				Vector2(scaleX * m_size.x / textureSize.x, scaleY * m_size.y / textureSize.y) * ratio
			);
		}
	}

	void GUIImage::SetTexture(Texture* value, bool setImageSize)
	{
		m_texture = value;
		if (setImageSize && m_texture)
		{
			Vector2Int textureSize = m_texture->GetSize();
			m_size = Vector2(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y));
		}
	}
}