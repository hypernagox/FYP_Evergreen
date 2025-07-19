#include "pch.h"
#include "MainMenuCharacterGUI.h"
#include "GameGUIFacade.h"
#include "TransitionOverlayGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void MainMenuCharacterGUI::OnInitialize()
{
	{
		m_panel = SceneObject::MakeShared();
		GetSceneObject()->AddChild(m_panel);
	}

	{
		auto backgroundObj = SceneObject::MakeShared();
		auto backgroundRenderer = backgroundObj->AddComponent<GUIImage>();
		backgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
		backgroundRenderer->SetColor(Color(0.0f, 0.0f, 0.0f, 0.8f));
		backgroundRenderer->SetSize(Vector2(8192.0f, 120.0f));
		backgroundObj->GetTransform()->SetLocalPosition(Vector3(0.0f, -660.0f, 0.0f));
		m_panel->AddChild(backgroundObj);
	}

	{
		m_nextButton = SceneObject::MakeShared();
		m_nextButton->GetTransform()->SetLocalPosition(Vector3(200.0f, -660.0f, 0.0f));

		auto buttonComponent = m_nextButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_arrow_R.png")));
		buttonComponent->SetSize(Vector2(50.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			SetSelectIndex((m_selectIndex + 1) % 3);
			});
		m_panel->AddChild(m_nextButton);
	}

	{
		m_prevButton = SceneObject::MakeShared();
		m_prevButton->GetTransform()->SetLocalPosition(Vector3(-200.0f, -660.0f, 0.0f));

		auto buttonComponent = m_prevButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_arrow_L.png")));
		buttonComponent->SetSize(Vector2(50.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			SetSelectIndex((m_selectIndex + 2) % 3);
			});
		m_panel->AddChild(m_prevButton);
	}

	{
		m_selectButton = SceneObject::MakeShared();
		m_selectButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -660.0f, 0.0f));

		auto buttonComponent = m_selectButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			EnterGame();
			});

		auto exitText = m_selectButton->AddComponent<GUIText>();
		exitText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		exitText->SetText(L"Select");
		exitText->SetRaycastTarget(false);
		m_panel->AddChild(m_selectButton);
	}
}

void MainMenuCharacterGUI::EnterGame()
{
	if (m_enterGameCallback)
	{
		m_enterGameCallback(m_selectIndex);
	}
}

void MainMenuCharacterGUI::SetSelectIndex(unsigned int index)
{
	m_selectIndex = index;
	if (m_characterShowCallback)
	{
		m_characterShowCallback(m_selectIndex);
	}
}
