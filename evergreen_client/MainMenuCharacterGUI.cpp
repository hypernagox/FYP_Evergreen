#include "pch.h"
#include "MainMenuCharacterGUI.h"

using namespace udsdx;

MainMenuCharacterGUI::MainMenuCharacterGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	{
		m_panel = std::make_shared<SceneObject>();
		object->AddChild(m_panel);
	}

	{
		m_nextButton = std::make_shared<SceneObject>();
		m_nextButton->GetTransform()->SetLocalPosition(Vector3(200.0f, -480.0f, 0.0f));

		auto buttonComponent = m_nextButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_arrow_R.png")));
		buttonComponent->SetSize(Vector2(50.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			SetSelectIndex((m_selectIndex + 1) % 3);
			});
		m_panel->AddChild(m_nextButton);
	}

	{
		m_prevButton = std::make_shared<SceneObject>();
		m_prevButton->GetTransform()->SetLocalPosition(Vector3(-200.0f, -480.0f, 0.0f));

		auto buttonComponent = m_prevButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_arrow_L.png")));
		buttonComponent->SetSize(Vector2(50.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			SetSelectIndex((m_selectIndex + 2) % 3);
			});
		m_panel->AddChild(m_prevButton);
	}

	{
		m_selectButton = std::make_shared<SceneObject>();
		m_selectButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -480.0f, 0.0f));

		auto buttonComponent = m_selectButton->AddComponent<GUIButton>();
		buttonComponent->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		buttonComponent->SetSize(Vector2(200.0f, 50.0f));
		buttonComponent->SetClickCallback([this]() {
			m_panel->SetActive(false);
			if (m_enterGameCallback)
			{
				m_enterGameCallback(m_selectIndex);
			}
			});

		auto exitText = m_selectButton->AddComponent<GUIText>();
		exitText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		exitText->SetText(L"Select");
		exitText->SetRaycastTarget(false);
		m_panel->AddChild(m_selectButton);
	}
}

void MainMenuCharacterGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
}

void MainMenuCharacterGUI::SetSelectIndex(unsigned int index)
{
	m_selectIndex = index;
	if (m_characterShowCallback)
	{
		m_characterShowCallback(m_selectIndex);
	}
}
