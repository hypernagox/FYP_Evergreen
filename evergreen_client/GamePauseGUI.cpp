#include "pch.h"
#include "GamePauseGUI.h"
#include "ChannelSwitchGUI.h"

using namespace udsdx;

GamePauseGUI::GamePauseGUI(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2::One * 8192.0f);
	object->AddChild(m_panel);

	{
		m_resumeButton = std::make_shared<SceneObject>();
		m_resumeButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -240.0f, 0.0f));

		auto buttonComponent = m_resumeButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));

		auto resumeText = m_resumeButton->AddComponent<GUIText>();
		resumeText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		resumeText->SetText(L"Resume");
		resumeText->SetRaycastTarget(false);
		m_panel->AddChild(m_resumeButton);
	}

	{
		m_channelSwitchButton = std::make_shared<SceneObject>();
		m_channelSwitchButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -300.0f, 0.0f));

		auto buttonComponent = m_channelSwitchButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			if (m_channelSwitchGUI)
			{
				m_channelSwitchGUI->SetActive(true);
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
		m_exitButton = std::make_shared<SceneObject>();
		m_exitButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -360.0f, 0.0f));

		auto buttonComponent = m_exitButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			if (m_exitGameCallback)
			{
				m_exitGameCallback();
			}
		});

		auto exitText = m_exitButton->AddComponent<GUIText>();
		exitText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		exitText->SetText(L"Exit");
		exitText->SetRaycastTarget(false);
		m_panel->AddChild(m_exitButton);
	}
}

void GamePauseGUI::SetTogglePauseCallback(std::function<void(bool)> callback)
{
	m_togglePauseCallback = callback;
	m_resumeButton->GetComponent<GUIButton>()->SetClickCallback([this]() {
		SetActivePanel(false);
		});
	SetActivePanel(false);
}

void GamePauseGUI::SetActivePanel(bool active)
{
	m_panel->SetActive(active);
	m_togglePauseCallback(active);
}

void GamePauseGUI::ToggleActivePanel()
{
	bool active = !m_panel->GetActive();
	SetActivePanel(active);
}
