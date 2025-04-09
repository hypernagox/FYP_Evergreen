#pragma once

#include "pch.h"
#include "gui_image.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIButton : public GUIImage
	{
	public:
		GUIButton(const std::shared_ptr<SceneObject>& object);

	public:
		void Render(RenderParam& param) override;
		void OnMouseRelease() override;

	public:
		void SetClickCallback(std::function<void()> clickCallback) { m_clickCallback = clickCallback; }

	private:
		std::function<void()> m_clickCallback = nullptr;
	};
}