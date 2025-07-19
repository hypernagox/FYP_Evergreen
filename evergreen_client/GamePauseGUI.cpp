#include "pch.h"
#include "GamePauseGUI.h"
#include "ChannelSwitchGUI.h"
#include "PopupGUIManager.h"
#include "GameGUIFacade.h"
#include "TransitionOverlayGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void GamePauseGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2::One * 8192.0f);
	GetSceneObject()->AddChild(m_panel);

	{
		m_resumeButton = SceneObject::MakeShared();
		m_resumeButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -240.0f, 0.0f));

		auto buttonComponent = m_resumeButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop();
		});

		auto resumeText = m_resumeButton->AddComponent<GUIText>();
		resumeText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		resumeText->SetText(L"Resume");
		resumeText->SetRaycastTarget(false);
		m_panel->AddChild(m_resumeButton);
	}

	{
		m_channelSwitchButton = SceneObject::MakeShared();
		m_channelSwitchButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -300.0f, 0.0f));

		auto buttonComponent = m_channelSwitchButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			if (m_channelSwitchGUI)
			{
				GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Append(m_channelSwitchGUI);
				m_channelSwitchGUI->GetComponent<ChannelSwitchGUI>()->SwitchChannelPage(0);
			}
		});

		auto channelText = m_channelSwitchButton->AddComponent<GUIText>();
		channelText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		channelText->SetText(L"Channels");
		channelText->SetRaycastTarget(false);
		m_panel->AddChild(m_channelSwitchButton);
	}

	{
		m_exitButton = SceneObject::MakeShared();
		m_exitButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -360.0f, 0.0f));

		auto buttonComponent = m_exitButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([this]() {
				m_exitGameCallback();
				},
				L"메인화면 이동 중 ...");
			});

		auto exitText = m_exitButton->AddComponent<GUIText>();
		exitText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		exitText->SetText(L"Return Main");
		exitText->SetRaycastTarget(false);
		m_panel->AddChild(m_exitButton);
	}
}