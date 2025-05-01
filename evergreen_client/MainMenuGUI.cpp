#include "MainMenuGUI.h"

using namespace udsdx;

MainMenuGUI::MainMenuGUI(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_backgroundImage = m_panel->AddComponent<GUIImage>();
	m_backgroundImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
	m_backgroundImage->SetSize(Vector2::One * 8192.0f);
	object->AddChild(m_panel);

	m_verticalPanel = std::make_shared<SceneObject>();
	m_verticalPanel->GetTransform()->SetLocalPosition(Vector3(-480.0f, 0.0f, 0.0f));
	auto verticalPanelRenderer = m_verticalPanel->AddComponent<GUIImage>();
	verticalPanelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
	verticalPanelRenderer->SetSize(Vector2(640.0f, 1080.0f));
	verticalPanelRenderer->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.5f));
	m_panel->AddChild(m_verticalPanel);

	m_titleImage = std::make_shared<SceneObject>();
	m_titleImage->GetTransform()->SetLocalPosition(Vector3(-480.0f, 200.0f, 0.0f));
	auto titleImageRenderer = m_titleImage->AddComponent<GUIImage>();
	titleImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\title.png")));
	titleImageRenderer->SetSize(Vector2(400.0f, 400.0f));
	m_panel->AddChild(m_titleImage);

	{
		m_playButton = std::make_shared<SceneObject>();
		m_playButton->GetTransform()->SetLocalPosition(Vector3(-480.0f, -300.0f, 0.0f));

		auto buttonComponent = m_playButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			m_panel->SetActive(false);
			if (m_enterGameCallback)
			{
				m_enterGameCallback();
			}
			});

		auto resumeText = m_playButton->AddComponent<GUIText>();
		resumeText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		resumeText->SetText(L"Play Game");
		resumeText->SetRaycastTarget(false);
		m_panel->AddChild(m_playButton);
	}

	{
		m_exitButton = std::make_shared<SceneObject>();
		m_exitButton->GetTransform()->SetLocalPosition(Vector3(-480.0f, -360.0f, 0.0f));

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
		exitText->SetText(L"Exit Game");
		exitText->SetRaycastTarget(false);
		m_panel->AddChild(m_exitButton);
	}
}

void MainMenuGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_elapsedTime += time.deltaTime;
	m_backgroundImage->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(1.0f - m_elapsedTime * 0.4f, 0.0f, 1.0f)));
}
