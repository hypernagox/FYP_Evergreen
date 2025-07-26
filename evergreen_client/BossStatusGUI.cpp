#include "pch.h"
#include "BossStatusGUI.h"

using namespace udsdx;

void BossStatusGUI::OnInitialize()
{
	m_iconObject = SceneObject::MakeShared();
	m_iconObject->GetTransform()->SetLocalPosition(Vector3(0.0f, 627.0f, 0.0f));
	auto iconRenderer = m_iconObject->AddComponent<GUIImage>();
	iconRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\boss\\icon_boss.png")));
	iconRenderer->SetSize(Vector2(185.0f, 185.0f));

	GetSceneObject()->AddChild(m_iconObject);

	m_panelObject = SceneObject::MakeShared();
	m_panelObject->GetTransform()->SetLocalPosition(Vector3(0.0f, 500.0f, 0.0f));
	auto panelRenderer = m_panelObject->AddComponent<GUIImage>();
	panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\boss\\boss_status.png")));
	panelRenderer->SetSize(Vector2(904.0f, 126.0f));

	GetSceneObject()->AddChild(m_panelObject);

	m_panelImpactObject = SceneObject::MakeShared();
	auto panelImpactRenderer = m_panelImpactObject->AddComponent<GUIImage>();
	panelImpactRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\boss\\boss_status_impact.png")));
	panelImpactRenderer->SetSize(Vector2(904.0f, 126.0f));
	panelImpactRenderer->SetFillType(GUIImage::FillType::FillHorizontalRight);

	m_panelObject->AddChild(m_panelImpactObject);

	m_panelFillObject = SceneObject::MakeShared();
	auto panelFillRenderer = m_panelFillObject->AddComponent<GUIImage>();
	panelFillRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\boss\\boss_status_fill.png")));
	panelFillRenderer->SetSize(Vector2(904.0f, 126.0f));
	panelFillRenderer->SetFillType(GUIImage::FillType::FillHorizontalRight);

	m_panelObject->AddChild(m_panelFillObject);
}

void BossStatusGUI::OnActive()
{
	m_appearTime = 0.0f;
}

void BossStatusGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_appearTime += time.deltaTime;
	m_hpImpactTime -= time.deltaTime;
	if (m_hpImpactTime <= 0.0f)
		m_hpImpactFraction = std::lerp(m_hpImpactFraction, m_hpFraction, time.deltaTime * 8.0f);

	m_panelImpactObject->GetComponent<GUIImage>()->SetFillAmount(std::min(m_hpImpactFraction, m_appearTime * 0.5f));
	m_panelFillObject->GetComponent<GUIImage>()->SetFillAmount(std::min(m_hpFraction, m_appearTime * 0.5f));
}
