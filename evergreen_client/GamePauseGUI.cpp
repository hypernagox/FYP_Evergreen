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
		m_verticalPanel[0] = SceneObject::MakeShared();
		m_verticalPanel[0]->GetTransform()->SetLocalPosition(Vector3(640.0f, 0.0f, 0.0f));
		auto verticalPanelRenderer = m_verticalPanel[0]->AddComponent<GUIImage>();
		verticalPanelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\alpha_gradient_black.png")));
		verticalPanelRenderer->SetSize(Vector2(1280.0f, 1440.0f));
		verticalPanelRenderer->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.75f));
		m_panel->AddChild(m_verticalPanel[0]);
	}

	{
		m_verticalPanel[1] = SceneObject::MakeShared();
		m_verticalPanel[1]->GetTransform()->SetLocalPosition(Vector3(-640.0f, 0.0f, 0.0f));
		auto verticalPanelRenderer = m_verticalPanel[1]->AddComponent<GUIImage>();
		verticalPanelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\alpha_gradient_black_flip.png")));
		verticalPanelRenderer->SetSize(Vector2(1280.0f, 1440.0f));
		verticalPanelRenderer->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.75f));
		m_panel->AddChild(m_verticalPanel[1]);
	}

	{
		auto titleImage = SceneObject::MakeShared();
		titleImage->GetTransform()->SetLocalPosition(Vector3(0.0f, 419.0f, 0.0f));
		auto titleImageRenderer = titleImage->AddComponent<GUIImage>();
		titleImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\title.png")));
		titleImageRenderer->SetSize(Vector2(780.0f, 520.0f));
		m_panel->AddChild(titleImage);
	}

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
		m_channelSwitchButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -320.0f, 0.0f));

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
		m_exitButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -400.0f, 0.0f));

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

void GamePauseGUI::OnActive()
{
	m_activeFactor = 0.0f;
	m_verticalPanel[0]->GetComponent<GUIImage>()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
	m_verticalPanel[1]->GetComponent<GUIImage>()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
}

void GamePauseGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_activeFactor = std::lerp(m_activeFactor, 1.0f, time.deltaTime * 8.0f);
	m_verticalPanel[0]->GetComponent<GUIImage>()->SetColor(Vector4(1.0f, 1.0f, 1.0f, m_activeFactor * 0.75f));
	m_verticalPanel[1]->GetComponent<GUIImage>()->SetColor(Vector4(1.0f, 1.0f, 1.0f, m_activeFactor * 0.75f));
}
