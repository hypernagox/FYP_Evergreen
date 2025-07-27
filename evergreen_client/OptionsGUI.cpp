#include "pch.h"
#include "OptionsGUI.h"
#include "GUISimpleButton.h"
#include "PopupGUIManager.h"

using namespace udsdx;

void OptionsGUI::OnInitialize()
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

	m_background = SceneObject::MakeShared();
	auto backgroundRenderer = m_background->AddComponent<GUIImage>();
	backgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	backgroundRenderer->SetSize(Vector2::One * 8192.0f);
	GetSceneObject()->AddChild(m_background);

	m_panel = SceneObject::MakeShared();
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\options_panel.png")), true);
	GetSceneObject()->AddChild(m_panel);

	auto exitButton = SceneObject::MakeShared();
	auto exitButtonRenderer = exitButton->AddComponent<GUISimpleButton>();
	exitButton->GetTransform()->SetLocalPosition(Vector3(179.0f, 74.0f, 0.0f));
	exitButtonRenderer->SetSize(Vector2(50.0f, 50.0f));
	exitButtonRenderer->SetClickCallback([&]() {
		GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
		});
	m_panel->AddChild(exitButton);
	std::wstring names[] = { L"Shadow Mapping", L"Screen Space AO", L"Motion Blur", L"FXAA" };

	for (int i = 0; i < static_cast<int>(m_optionButtons.size()); ++i)
	{
		float y = 20.0f - i * 32.0f;
		
		auto optionText = SceneObject::MakeShared();
		optionText->GetTransform()->SetLocalPosition(Vector3(-200.0f, y, 0.0f));
		auto textRenderer = optionText->AddComponent<GUIText>();
		textRenderer->SetFont(font);
		textRenderer->SetText(names[i]);
		textRenderer->SetAlignment(GUIText::Alignment::Left);

		m_optionButtons[i] = SceneObject::MakeShared();
		m_optionButtons[i]->GetTransform()->SetLocalPosition(Vector3(140.0f, y, 0.0f));
		auto buttonRenderer = m_optionButtons[i]->AddComponent<GUISimpleButton>();
		buttonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonRenderer->SetSize(Vector2(100.0f, 30.0f));
		buttonRenderer->SetClickCallback([this, i]() { OnButtonClick(i); });

		m_optionStatusTexts[i] = SceneObject::MakeShared();
		auto statusTextRenderer = m_optionStatusTexts[i]->AddComponent<GUIText>();
		statusTextRenderer->SetFont(font);
		statusTextRenderer->SetText(L"ON");
		statusTextRenderer->SetRaycastTarget(false);

		m_panel->AddChild(optionText);
		m_panel->AddChild(m_optionButtons[i]);
		m_optionButtons[i]->AddChild(m_optionStatusTexts[i]);

		// Todo: Load saved states for options
		m_optionStates[i] = true; // Initialize all options to ON state
	}
}

void OptionsGUI::OnActive()
{
	m_panel->GetTransform()->SetLocalPositionY(-50.0f);
}

void OptionsGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_panel->GetTransform()->SetLocalPositionY(std::lerp(m_panel->GetTransform()->GetLocalPosition().y, 0.0f, time.deltaTime * 16.0f));
}

void OptionsGUI::OnButtonClick(int index)
{
	m_optionStates[index] = !m_optionStates[index];
	auto statusTextRenderer = m_optionStatusTexts[index]->GetComponent<GUIText>();
	statusTextRenderer->SetText(m_optionStates[index] ? L"ON" : L"OFF");
	auto core = INSTANCE(Core);

	switch (index)
	{
	case 0:
		core->GetRenderOptionsRef().DrawShadowMap = m_optionStates[index];
		break;
	case 1:
		core->GetRenderOptionsRef().DrawSSAO = m_optionStates[index];
		core->PrepareDirectCommandList();
		core->GetScreenSpaceAO()->ClearSSAOMap(core->GetCommandList());
		core->ExecuteAndFlushDirectCommandList();
		break;
	case 2:
		core->GetRenderOptionsRef().DrawMotionBlur = m_optionStates[index];
		break;
	case 3:
		core->GetRenderOptionsRef().DrawFXAA = m_optionStates[index];
		break;
	}
}
