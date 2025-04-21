#include "pch.h"
#include "LogFloatGUI.h"

using namespace udsdx;

LogFloatGUI::LogFloatGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(-640.0f, -240.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
	object->AddChild(m_panel);

	m_floatText = std::make_shared<SceneObject>();
	auto floatText = m_floatText->AddComponent<GUIText>();
	floatText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_floatText->GetTransform()->SetLocalPosition(Vector3(-230.0f, -150.0f, 0.0f));
	floatText->SetRaycastTarget(false);
	floatText->SetAlignment(GUIText::Alignment::LowerLeft);
	m_panel->AddChild(m_floatText);
}

void LogFloatGUI::AddText(const std::wstring& text)
{
	auto floatText = m_floatText->GetComponent<GUIText>();
	std::wstring currentText = floatText->GetText();

	// Limit the number of lines to 10
	const size_t maxLines = 10;
	size_t currentLines = std::count(currentText.begin(), currentText.end(), L'\n');
	if (currentLines >= maxLines)
	{
		size_t pos = currentText.find(L'\n');
		if (pos != std::wstring::npos)
		{
			currentText.erase(0, pos + 1);
		}
	}
	std::wstring newText = currentText + L"\n" + text;
	floatText->SetText(newText);
}
