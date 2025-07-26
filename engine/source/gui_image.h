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
		enum class FillType
		{
			Default,
			FillHorizontalRight,
			FillHorizontalLeft,
			FillVerticalUp,
			FillVerticalDown,
		};
	public:
		void Render(RenderParam& param) override;

	public:
		Texture* GetTexture() const { return m_texture; }
		void SetTexture(Texture* value, bool setImageSize = false);
		FillType GetFillType() const { return m_fillType; }
		void SetFillType(FillType type) { m_fillType = type; }
		float GetFillAmount() const { return m_fillAmount; }
		void SetFillAmount(float amount) { m_fillAmount = std::clamp(amount, 0.0f, 1.0f); }

	protected:
		Texture* m_texture = nullptr;
		FillType m_fillType = FillType::Default;
		float m_fillAmount = 1.0f;
	};
}