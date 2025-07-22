#include "pch.h"
#include "MonsterHPPanel.h"
#include "WorldSpaceGUI.h"
#include "GameScene.h"

void MonsterHPPanel::OnInitialize()
{
	m_panelObject = SceneObject::MakeShared();
	auto panelRenderer = m_panelObject->AddComponent<GUIImage>();
	panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\monster_status\\panel.png")), true);

	auto panelWorldTransform = AddComponent<WorldSpaceGUI>();
	panelWorldTransform->SetSourceTransform(GetTransform());
	panelWorldTransform->SetTargetObject(m_panelObject);
	panelWorldTransform->SetAngleRange(30.0f);
	panelWorldTransform->SetViewFar(30.0f);
	panelWorldTransform->SetWorldOffset(Vector3::Up * 2.0f);
	panelWorldTransform->SetScreenOffset(Vector3(117.0f, 40.0f, 0.0f));

	m_levelObject = SceneObject::MakeShared();
	auto levelRenderer = m_levelObject->AddComponent<GUIText>();
	levelRenderer->SetText(L"10");
	levelRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	m_levelObject->GetTransform()->SetLocalPosition(Vector3(-62.0f, 14.0f, 0.0f));

	m_textObject = SceneObject::MakeShared();
	auto textRenderer = m_textObject->AddComponent<GUIText>();
	textRenderer->SetText(L"Monster Name");
	textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	textRenderer->SetAlignment(GUIText::Alignment::Left);
	m_textObject->GetTransform()->SetLocalPosition(Vector3(-35.0f, 18.0f, 0.0f));

	m_hpBarImpactObject = SceneObject::MakeShared();
	auto hpBarImpactRenderer = m_hpBarImpactObject->AddComponent<GUIImage>();
	hpBarImpactRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\monster_status\\hpimpact.png")), true);

	m_hpBarObject = SceneObject::MakeShared();
	auto hpBarRenderer = m_hpBarObject->AddComponent<GUIImage>();
	hpBarRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\monster_status\\hpfill.png")), true);

	m_panelObject->AddChild(m_levelObject);
	m_panelObject->AddChild(m_textObject);
	m_panelObject->AddChild(m_hpBarImpactObject);
	m_panelObject->AddChild(m_hpBarObject);
	m_panelObject->SetActive(false);
}

void MonsterHPPanel::OnAttach()
{
	auto scene = GetSceneObject()->GetScene();
	if (scene != nullptr)
	{
		auto gameScene = dynamic_cast<GameScene*>(scene);
		if (gameScene != nullptr)
		{
			gameScene->AddInterfaceObject(m_panelObject);
		}
	}
}

void MonsterHPPanel::OnDetach()
{
	m_panelObject->RemoveFromParent();
}

void MonsterHPPanel::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_hpImpactTime -= time.deltaTime;
	if (m_hpImpactTime <= 0.0f)
		m_hpImpactFraction = std::lerp(m_hpImpactFraction, m_hpFraction, time.deltaTime * 8.0f);

	m_hpBarImpactObject->GetTransform()->SetLocalPosition(Vector3(std::lerp(49.0f - 84.0f, 49.0f, m_hpImpactFraction), 2.0f, 0.0f));
	m_hpBarImpactObject->GetComponent<GUIImage>()->SetSize(Vector2(168.0f * m_hpImpactFraction, 8.0f));

	m_hpBarObject->GetTransform()->SetLocalPosition(Vector3(std::lerp(49.0f - 84.0f, 49.0f, m_hpFraction), 2.0f, 0.0f));
	m_hpBarObject->GetComponent<GUIImage>()->SetSize(Vector2(168.0f * m_hpFraction, 8.0f));
}

void MonsterHPPanel::SetText(std::wstring_view text)
{
	m_textObject->GetComponent<GUIText>()->SetText(text.data());
}
