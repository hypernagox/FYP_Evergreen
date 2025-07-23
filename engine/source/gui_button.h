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
		void Render(RenderParam& param) override;
		void OnMouseRelease(int mouse) override;

	public:
		bool GetInteractable() const { return m_interactable; }
		void SetClickCallback(std::function<void()> clickCallback) { m_clickCallback = clickCallback; }
		void SetInteractable(bool interactable) { m_interactable = interactable; }

	private:
		std::function<void()> m_clickCallback = nullptr;
		bool m_interactable = true;
	};
}