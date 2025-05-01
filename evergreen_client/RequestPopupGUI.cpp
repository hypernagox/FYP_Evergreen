#include "pch.h"
#include "RequestPopupGUI.h"

using namespace udsdx;

RequestPopupGUI::RequestPopupGUI(const std::shared_ptr<udsdx::SceneObject>& object) : udsdx::Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(0.0f, -240.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_0_entry.png")));
	uiRenderer->SetSize(Vector2(420.0f, 240.0f));
	object->AddChild(m_panel);

	m_titleText = std::make_shared<SceneObject>();
	auto titleTextRenderer = m_titleText->AddComponent<GUIText>();
	titleTextRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_titleText->GetTransform()->SetLocalPosition(Vector3(0.0f, 75.0f, 0.0f));
	titleTextRenderer->SetRaycastTarget(false);
	titleTextRenderer->SetAlignment(GUIText::Alignment::Center);
	titleTextRenderer->SetText(L"####");
	m_panel->AddChild(m_titleText);

	m_text = std::make_shared<SceneObject>();
	auto textRenderer = m_text->AddComponent<GUIText>();
	textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_text->GetTransform()->SetLocalPosition(Vector3(0.0f, 20.0f, 0.0f));
	textRenderer->SetRaycastTarget(false);
	textRenderer->SetAlignment(GUIText::Alignment::UpperCenter);
	textRenderer->SetText(L"####");
	m_panel->AddChild(m_text);

	m_acceptButton = std::make_shared<SceneObject>();
	auto acceptButtonRenderer = m_acceptButton->AddComponent<GUIButton>();
	m_acceptButton->GetTransform()->SetLocalPosition(Vector3(-100.0f, -80.0f, 0.0f));
	acceptButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_select.png")));
	acceptButtonRenderer->SetSize(Vector2(160.0f, 60.0f));
	acceptButtonRenderer->SetClickCallback([this]() {
		if (m_onAccept)
			m_onAccept();
		m_panel->SetActive(false);
	});
	m_panel->AddChild(m_acceptButton);

	auto acceptButtonText = m_acceptButton->AddComponent<GUIText>();
	acceptButtonText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	acceptButtonText->SetRaycastTarget(false);
	acceptButtonText->SetAlignment(GUIText::Alignment::Center);
	acceptButtonText->SetText(L"Accept");

	m_cancelButton = std::make_shared<SceneObject>();
	auto cancelButtonRenderer = m_cancelButton->AddComponent<GUIButton>();
	m_cancelButton->GetTransform()->SetLocalPosition(Vector3(100.0f, -80.0f, 0.0f));
	cancelButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_select.png")));
	cancelButtonRenderer->SetSize(Vector2(160.0f, 60.0f));
	cancelButtonRenderer->SetClickCallback([this]() {
		if (m_onCancel)
			m_onCancel();
		m_panel->SetActive(false);
	});
	m_panel->AddChild(m_cancelButton);

	auto cancelButtonText = m_cancelButton->AddComponent<GUIText>();
	cancelButtonText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	cancelButtonText->SetRaycastTarget(false);
	cancelButtonText->SetAlignment(GUIText::Alignment::Center);
	cancelButtonText->SetText(L"Cancel");

	m_panel->SetActive(false);
}

void RequestPopupGUI::ShowPopup(std::wstring_view title, std::wstring_view contents, const std::function<void()>& onAccept, const std::function<void()>& onCancel)
{
	m_panel->SetActive(true);
	m_titleText->GetComponent<GUIText>()->SetText(title.data());
	m_text->GetComponent<GUIText>()->SetText(contents.data());

	m_onAccept = onAccept;
	m_onCancel = onCancel;
}
