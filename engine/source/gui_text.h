#pragma once

#include "component.h"
#include "gui_element.h"

namespace udsdx
{
	class Scene;
	class Texture;
	class Font;

	class GUIText : public GUIElement
	{
	public:
		enum class Alignment
		{
			UpperLeft,
			UpperCenter,
			UpperRight,
			Left,
			Center,
			Right,
			LowerLeft,
			LowerCenter,
			LowerRight
		};

	public:
		void Render(RenderParam& param) override;

	public:
		std::wstring GetText() const { return m_text; }
		void SetText(const std::wstring& value) { m_text = value; }

		Font* GetFont() const { return m_font; }
		void SetFont(Font* value) { m_font = value; }

		Alignment GetAlignment() const { return m_alignment; }
		void SetAlignment(Alignment value) { m_alignment = value; }

	private:
		Font* m_font = nullptr;
		std::wstring m_text;
		Alignment m_alignment = Alignment::Center;
	};
}