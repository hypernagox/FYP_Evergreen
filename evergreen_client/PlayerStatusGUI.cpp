#include "pch.h"
#include "PlayerStatusGUI.h"

using namespace udsdx;

void PlayerStatusGUI::OnInitialize()
{
    {
        m_healthBackground = SceneObject::MakeShared();
        auto uiRenderer = m_healthBackground->AddComponent<GUIImage>();
        m_healthBackground->GetTransform()->SetLocalPosition(Vector3(-720.0f, -630.0f, 0.0f));
        uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_background.png")), true);
    }

    {
        m_healthFill = SceneObject::MakeShared();
        m_healthFillRenderer = m_healthFill->AddComponent<GUIImage>();
        m_healthFill->GetTransform()->SetLocalPosition(Vector3(-702.5f, -630.0f, 0.0f));
        m_healthFillRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_fill.png")), true);
        m_healthFillRenderer->SetFillType(GUIImage::FillType::FillHorizontalRight);
        m_healthFillWidthCache = static_cast<float>(m_healthFillRenderer->GetSize().x);
    }

    {
        m_textObj = SceneObject::MakeShared();
        m_textObj->GetTransform()->SetLocalPosition(Vector3(-702.5f, -630.0f, 0.0f));
        m_textRenderer = m_textObj->AddComponent<GUIText>();
        m_textRenderer->SetText(L"## / ##");
        m_textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
    }

    GetSceneObject()->AddChild(m_healthBackground);
    GetSceneObject()->AddChild(m_healthFill);
    GetSceneObject()->AddChild(m_textObj);

    SetCurrentHealth(m_maxHealth);
}

void PlayerStatusGUI::SetMaxHealth(int value)
{
	m_maxHealth = value;
}

void PlayerStatusGUI::SetCurrentHealth(int value)
{
	m_currentHealth = value;
    float factor = std::clamp(static_cast<float>(m_currentHealth) / static_cast<float>(m_maxHealth), 0.0f, 1.0f);
    m_healthFillRenderer->SetFillAmount(factor);
    m_textRenderer->SetText(std::format(L"{0:02} / {1:02}", m_currentHealth, m_maxHealth));
}

void PlayerStatusGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
    Component::Update(time, scene);
}
