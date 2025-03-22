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
		const float height = 1080.0f;
		const float width = height * param.AspectRatio;
		float ratio = param.Viewport.Height / height;
		Vector3 position = GetTransform()->GetWorldPosition() * Vector3(ratio, -ratio, 1.0f) + Vector3(param.Viewport.Width / 2.0f, param.Viewport.Height / 2.0f, 0.0f);
		if (m_texture != nullptr)
		{
			param.SpriteBatchNonPremultipliedAlpha->Draw(m_texture->GetSrvGpu(), XMUINT2(m_size.x, m_size.y), position, nullptr, Colors::White, 0.0f, Vector2(m_size.x, m_size.y) * 0.5f, Vector2::One * ratio);
		}
	}

	void GUIImage::SetTexture(Texture* value, bool setImageSize)
	{
		m_texture = value;
		if (setImageSize && m_texture)
		{
			m_size = m_texture->GetSize();
		}
	}
}