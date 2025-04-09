#pragma once

#include "pch.h"
#include "gui_element.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIImage : public GUIElement
	{
	public:
		GUIImage(const std::shared_ptr<SceneObject>& object);

	public:
		void Render(RenderParam& param) override;

	public:
		Texture* GetTexture() const { return m_texture; }
		void SetTexture(Texture* value, bool setImageSize = false);

	private:
		Texture* m_texture = nullptr;
	};
}