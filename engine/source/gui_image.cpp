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
		float ratio = param.Viewport.Height / RefScreenSize.y;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		if (m_texture != nullptr)
		{
			Matrix4x4 m = GetTransform()->GetWorldSRTMatrix();

			// 각 축 벡터 추출
			Vector3 xAxis(m._11, m._21, m._31);  // 첫 번째 열
			Vector3 yAxis(m._12, m._22, m._32);  // 두 번째 열

			// 벡터 길이로 스케일 계산
			float scaleX = xAxis.Length();
			float scaleY = yAxis.Length();

			Vector2Int textureSize = m_texture->GetSize();
			param.SpriteBatchNonPremultipliedAlpha->Draw(
				m_texture->GetSrvGpu(),
				XMUINT2(textureSize.x, textureSize.y),
				position,
				nullptr,
				m_color,
				0.0f,
				Vector2(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)) * 0.5f,
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