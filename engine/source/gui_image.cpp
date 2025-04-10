#include "pch.h"
#include "gui_image.h"
#include "scene.h"
#include "core.h"
#include "shader_compile.h"
#include "transform.h"
#include "texture.h"

namespace udsdx
{
	GUIImage::GUIImage(const std::shared_ptr<SceneObject>& object) : GUIElement(object)
	{
	}

	void GUIImage::Render(RenderParam& param)
	{
		float ratio = param.Viewport.Height / RefScreenSize.y;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		if (m_texture != nullptr)
		{
			Vector2Int textureSize = m_texture->GetSize();
			param.SpriteBatchNonPremultipliedAlpha->Draw(
				m_texture->GetSrvGpu(),
				XMUINT2(textureSize.x, textureSize.y),
				position,
				nullptr,
				Colors::White,
				0.0f,
				Vector2(static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)) * 0.5f,
				Vector2(m_size.x / textureSize.x, m_size.y / textureSize.y) * ratio
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