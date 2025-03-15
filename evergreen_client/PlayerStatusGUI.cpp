#include "pch.h"
#include "PlayerStatusGUI.h"

using namespace udsdx;

PlayerStatusGUI::PlayerStatusGUI(const std::shared_ptr<udsdx::SceneObject>& object)
	: Component(object)
{
    {
        m_healthBackground = std::make_shared<SceneObject>();
        auto uiRenderer = m_healthBackground->AddComponent<GUIImage>();
        m_healthBackground->GetTransform()->SetLocalPosition(Vector3(-640.0f, -480.0f, 0.0f));
        uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_background.png")), true);
    }

    {
        m_healthFill = std::make_shared<SceneObject>();
        m_healthFillRenderer = m_healthFill->AddComponent<GUIImage>();
        m_healthFill->GetTransform()->SetLocalPosition(Vector3(-622.5f, -480.0f, 0.0f));
        m_healthFillRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_fill.png")), true);
        m_healthFillWidthCache = m_healthFillRenderer->GetSize().x;
    }

    object->AddChild(m_healthBackground);
    object->AddChild(m_healthFill);

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
    Vector2 size = m_healthFillRenderer->GetSize();
    size.x = m_healthFillWidthCache * factor;
    m_healthFillRenderer->SetSize(size);
    m_healthFill->GetTransform()->SetLocalPosition(Vector3(-622.5f + (size.x - m_healthFillWidthCache) / 2.0f, -480.0f, 0.0f));
}

void PlayerStatusGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
    Component::Update(time, scene);
}
