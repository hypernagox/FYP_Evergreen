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
		GUIText(const std::shared_ptr<SceneObject>& object);

	public:
		void Render(RenderParam& param) override;

	public:
		std::wstring GetText() const { return m_text; }
		void SetText(const std::wstring& value) { m_text = value; }

		Font* GetFont() const { return m_font; }
		void SetFont(Font* value) { m_font = value; }

	private:
		Font* m_font = nullptr;
		std::wstring m_text;
	};
}