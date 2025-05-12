#include "pch.h"
#include "InteractionFloatGUI.h"
#include "GameScene.h"

using namespace udsdx;

InteractionFloatGUI::InteractionFloatGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(0.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
	uiRenderer->SetSize(Vector2(200.0f, 40.0f));
	object->AddChild(m_panel);	

	m_interactionText = std::make_shared<SceneObject>();
	auto interactionText = m_interactionText->AddComponent<GUIText>();
	interactionText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_interactionText->GetTransform()->SetLocalPosition(Vector3(90.0f, 0.0f, 0.0f));
	interactionText->SetRaycastTarget(false);
	interactionText->SetText(L"####");
	interactionText->SetAlignment(GUIText::Alignment::Right);
	m_panel->AddChild(m_interactionText);

	m_interactionIcon = std::make_shared<SceneObject>();
	auto interactionIconRenderer = m_interactionIcon->AddComponent<GUIImage>();
	m_interactionIcon->GetTransform()->SetLocalPosition(Vector3(-80.0f, 0.0f, 0.0f));
	interactionIconRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\key_E.png")));
	interactionIconRenderer->SetSize(Vector2(40, 40));
	m_panel->AddChild(m_interactionIcon);
}

void InteractionFloatGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	auto gameScene = dynamic_cast<GameScene*>(&scene);
	if (gameScene != nullptr)
	{
		auto camera = gameScene->GetMainCamera();
		Vector3 viewPos = Vector3::Transform(m_targetPos + Vector3::Up, camera->GetViewMatrix());
		if (viewPos.z > 0.0f)
		{
			m_panel->SetActive(true);
			float aspectRatio = INSTANCE(Core)->GetAspectRatio();
			Vector3 screenPos = Vector3::Transform(viewPos, camera->GetProjMatrix(aspectRatio));
			screenPos.x *= GUIElement::RefScreenSize.y * aspectRatio * 0.5f;
			screenPos.y *= GUIElement::RefScreenSize.y * 0.5f;
			GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f));
		}
		else
			m_panel->SetActive(false);
	}
}

void InteractionFloatGUI::SetText(const std::wstring& text)
{
	auto textRenderer = m_interactionText->GetComponent<GUIText>();
	textRenderer->SetText(text);
}
