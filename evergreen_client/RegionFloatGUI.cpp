#include "pch.h"
#include "RegionFloatGUI.h"

using namespace udsdx;

void RegionFloatGUI::OnInitialize()
{
	m_backgroundObject = SceneObject::MakeShared();
	m_backgroundObject->GetTransform()->SetLocalPositionY(320.0f);
	auto backgroundImage = m_backgroundObject->AddComponent<GUIImage>();
	backgroundImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\float_background.png")), true);
	backgroundImage->SetRaycastTarget(false);

	m_textObject = SceneObject::MakeShared();
	m_textObject->GetTransform()->SetLocalScale(2.0f);
	auto textRenderer = m_textObject->AddComponent<GUIText>();
	textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	textRenderer->SetText(L"# # # #");
	textRenderer->SetRaycastTarget(false);

	GetSceneObject()->AddChild(m_backgroundObject);
	m_backgroundObject->AddChild(m_textObject);

	m_elapsedTime = 10.0f;
}

void RegionFloatGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_elapsedTime += time.deltaTime;
	float visibleFactor = std::clamp(std::min(m_elapsedTime, 1.0f) - std::max(m_elapsedTime - 5.0f, 0.0f), 0.0f, 1.0f);
	m_backgroundObject->GetComponent<GUIImage>()->SetColor(udsdx::Color(1.0f, 1.0f, 1.0f, visibleFactor));
	m_textObject->GetComponent<GUIText>()->SetColor(udsdx::Color(1.0f, 1.0f, 1.0f, visibleFactor));
}

void RegionFloatGUI::Float(std::wstring_view content)
{
	m_elapsedTime = 0.0f;
	std::wstring formattedContent;
	// Insert blank space for each character to make it wide
	for (const auto& ch : content)
	{
		formattedContent += std::wstring(1, ch) + L" ";
	}
	formattedContent.pop_back(); // Remove the last space
	m_textObject->GetComponent<GUIText>()->SetText(formattedContent);
}
