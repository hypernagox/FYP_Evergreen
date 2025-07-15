#include "pch.h"
#include "MainMenuGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void MainMenuGUI::OnInitialize()
{
	m_panel = std::make_shared<SceneObject>();
	m_backgroundImage = m_panel->AddComponent<GUIImage>();
	m_backgroundImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
	m_backgroundImage->SetSize(Vector2::One * 8192.0f);
	GetSceneObject()->AddChild(m_panel);

	auto verticalPanelFiller = std::make_shared<SceneObject>();
	verticalPanelFiller->GetTransform()->SetLocalPosition(Vector3(-2640.0f, 0.0f, 0.0f));
	auto fillerComponent = verticalPanelFiller->AddComponent<GUIImage>();
	fillerComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
	fillerComponent->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.8f));
	fillerComponent->SetSize(Vector2(2560.0f, 1440.0f));
	m_panel->AddChild(verticalPanelFiller);

	m_verticalPanel = std::make_shared<SceneObject>();
	m_verticalPanel->GetTransform()->SetLocalPosition(Vector3(-720.0f, 0.0f, 0.0f));
	auto verticalPanelRenderer = m_verticalPanel->AddComponent<GUIImage>();
	verticalPanelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\alpha_gradient_black.png")));
	verticalPanelRenderer->SetSize(Vector2(1280.0f, 1440.0f));
	verticalPanelRenderer->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.8f));
	m_panel->AddChild(m_verticalPanel);

	m_titleImage = std::make_shared<SceneObject>();
	m_titleImage->GetTransform()->SetLocalPosition(Vector3(-794.0f, 419.0f, 0.0f));
	auto titleImageRenderer = m_titleImage->AddComponent<GUIImage>();
	titleImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\title.png")));
	titleImageRenderer->SetSize(Vector2(780.0f, 440.0f));
	m_panel->AddChild(m_titleImage);

	m_versionText = std::make_shared<SceneObject>();
	m_versionText->GetTransform()->SetLocalPosition(Vector3(-1260.0f, -700.0f, 0.0f));
	auto versionText = m_versionText->AddComponent<GUIText>();
	versionText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	versionText->SetText(L"Version 0.1.1\nProduction of Tech University Korea");
	versionText->SetAlignment(GUIText::Alignment::LowerLeft);
	m_panel->AddChild(m_versionText);

	{
		m_playButton = std::make_shared<SceneObject>();
		m_playButton->GetTransform()->SetLocalPosition(Vector3(-1040.0f, -270.0f, 0.0f));

		auto buttonComponent = m_playButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(245.0f, 65.0f));
		buttonComponent->SetClickCallback([this]() {
			if (m_enterGameCallback)
			{
				m_enterGameCallback();
			}
			});

		auto resumeText = m_playButton->AddComponent<GUIText>();
		resumeText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		resumeText->SetText(L"Play Multiplayer");
		resumeText->SetRaycastTarget(false);
		m_panel->AddChild(m_playButton);
	}

	{
		m_optionsButton = std::make_shared<SceneObject>();
		m_optionsButton->GetTransform()->SetLocalPosition(Vector3(-1040.0f, -365.0f, 0.0f));

		auto buttonComponent = m_optionsButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(245.0f, 65.0f));

		auto resumeText = m_optionsButton->AddComponent<GUIText>();
		resumeText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		resumeText->SetText(L"Options");
		resumeText->SetRaycastTarget(false);
		m_panel->AddChild(m_optionsButton);
	}

	{
		m_exitButton = std::make_shared<SceneObject>();
		m_exitButton->GetTransform()->SetLocalPosition(Vector3(-1040.0f, -460.0f, 0.0f));

		auto buttonComponent = m_exitButton->AddComponent<GUISimpleButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(245.0f, 65.0f));
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

	float x = GUIElement::RefScreenSize.y * INSTANCE(Core)->GetAspectRatio();
	m_versionText->GetTransform()->SetLocalPosition(Vector3(x * -0.5f + 10.0f, GUIElement::RefScreenSize.y * -0.5f + 10.0f, 0.0f));
}
