#include "pch.h"
#include "PlayerStatusGUI.h"

using namespace udsdx;

void PlayerStatusGUI::OnInitialize()
{
    {
        m_healthBackground = SceneObject::MakeShared();
        auto uiRenderer = m_healthBackground->AddComponent<GUIImage>();
        m_healthBackground->GetTransform()->SetLocalPosition(Vector3(-665.0f, -630.0f, 0.0f));
        uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_background.png")), true);
    }

    {
        m_healthFill = SceneObject::MakeShared();
        m_healthFillRenderer = m_healthFill->AddComponent<GUIImage>();
        m_healthFill->GetTransform()->SetLocalPosition(Vector3(-647.5f, -630.0f, 0.0f));
        m_healthFillRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\health_fill.png")), true);
        m_healthFillRenderer->SetFillType(GUIImage::FillType::FillHorizontalRight);
        m_healthFillWidthCache = static_cast<float>(m_healthFillRenderer->GetSize().x);
    }

    {
        m_textObj = SceneObject::MakeShared();
        m_textObj->GetTransform()->SetLocalPosition(Vector3(-647.5f, -630.0f, 0.0f));
        m_textRenderer = m_textObj->AddComponent<GUIText>();
        m_textRenderer->SetText(L"## / ##");
        m_textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
    }

    {
        for (int skillIndex = 0; skillIndex < NumSkill; ++skillIndex)
        {
            m_skillBackgrounds[skillIndex] = SceneObject::MakeShared();
            m_skillBackgrounds[skillIndex]->GetTransform()->SetLocalPosition(Vector3(667.0f + 90.0f * skillIndex, -626.0f, 0.0f));
            auto uiRenderer = m_skillBackgrounds[skillIndex]->AddComponent<GUIImage>();
            uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\skill_blank.png")), true);
            uiRenderer->SetSize(Vector2(90.0f, 90.0f));

            m_skillFills[skillIndex] = SceneObject::MakeShared();
            auto skillFillRenderer = m_skillFills[skillIndex]->AddComponent<GUIImage>();
            skillFillRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\skill_" + std::to_wstring(skillIndex + 1) + L".png")), true);
            skillFillRenderer->SetFillType(GUIImage::FillType::FillVerticalUp);
            skillFillRenderer->SetSize(Vector2(90.0f, 90.0f));

            m_skillFractions[skillIndex] = 1.0f;
            m_skillReady[skillIndex] = true;

            GetSceneObject()->AddChild(m_skillBackgrounds[skillIndex]);
            m_skillBackgrounds[skillIndex]->AddChild(m_skillFills[skillIndex]);
        }
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
    for (int skillIndex = 0; skillIndex < NumSkill; ++skillIndex)
    {
        GUIImage* imageComponent = m_skillFills[skillIndex]->GetComponent<GUIImage>();
        if (!m_skillReady[skillIndex] && m_skillFractions[skillIndex] >= 1.0f)
        {
            m_skillFractions[skillIndex] = 1.0f;
			m_skillReady[skillIndex] = true; // Skill is ready again
            m_skillBackgrounds[skillIndex]->GetTransform()->SetLocalScale(Vector3(1.0f, 1.25f, 1.0f));
		}
        else if (m_skillReady[skillIndex])
        {
            Transform* skillTransform = m_skillBackgrounds[skillIndex]->GetTransform();
            skillTransform->SetLocalScale(Vector3::Lerp(skillTransform->GetLocalScale(), Vector3::One, time.deltaTime * 8.0f));
        }
        else
        {
            m_skillFractions[skillIndex] += time.deltaTime / 0.5f;
        }
        imageComponent->SetFillAmount(m_skillFractions[skillIndex]);
	}

    Component::Update(time, scene);
}

void PlayerStatusGUI::UseSkill(const int skillIndex)
{
	m_skillReady[skillIndex] = false;
	m_skillFractions[skillIndex] = 0.0f;
}
