#include "pch.h"
#include "InteractionFloatGUI.h"
#include "GameScene.h"
#include "WorldSpaceGUI.h"

using namespace udsdx;

void InteractionFloatGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	m_panel->GetTransform()->SetLocalPosition(Vector3(0.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
	uiRenderer->SetSize(Vector2(200.0f, 40.0f));
	GetSceneObject()->AddChild(m_panel);

	auto panelWorldTransform = AddComponent<WorldSpaceGUI>();
	panelWorldTransform->SetTargetObject(m_panel);

	m_interactionText = SceneObject::MakeShared();
	auto interactionText = m_interactionText->AddComponent<GUIText>();
	interactionText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_interactionText->GetTransform()->SetLocalPosition(Vector3(90.0f, 0.0f, 0.0f));
	interactionText->SetRaycastTarget(false);
	interactionText->SetText(L"####");
	interactionText->SetAlignment(GUIText::Alignment::Right);
	m_panel->AddChild(m_interactionText);

	m_interactionIcon = SceneObject::MakeShared();
	auto interactionIconRenderer = m_interactionIcon->AddComponent<GUIImage>();
	m_interactionIcon->GetTransform()->SetLocalPosition(Vector3(-80.0f, 0.0f, 0.0f));
	interactionIconRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\key_E.png")));
	interactionIconRenderer->SetSize(Vector2(40, 40));
	m_panel->AddChild(m_interactionIcon);
}

void InteractionFloatGUI::OnActive()
{
	m_interactionSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\blip.wav"))->CreateInstance();
	m_interactionSound->SetVolume(0.5f);
	m_interactionSound->Play();
	m_panel->GetTransform()->SetLocalScale(Vector3(0.0f, 1.0f, 1.0f));
}

void InteractionFloatGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	GetComponent<WorldSpaceGUI>()->SetWorldOffset(m_targetPos + Vector3::Up);
	float x = m_panel->GetTransform()->GetLocalScale().x;
	m_panel->GetTransform()->SetLocalScale(Vector3(std::lerp(x, 1.0f, time.deltaTime * 8.0f), 1.0f, 1.0f));
}

void InteractionFloatGUI::SetText(const std::wstring& text)
{
	auto textRenderer = m_interactionText->GetComponent<GUIText>();
	textRenderer->SetText(text);
}
