#include "pch.h"
#include "PlayerQuickSlotGUI.h"

using namespace udsdx;

PlayerQuickSlotGUI::PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
    for (int i = 0; i < NUM_SLOTS; i++)
    {
		m_slotBackground[i] = std::make_shared<SceneObject>();
		auto uiRenderer = m_slotBackground[i]->AddComponent<GUIButton>();
		m_slotBackground[i]->GetTransform()->SetLocalPosition(Vector3(640.0f + 120.0f * i, -400.0f, 0.0f));
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(std::format(L"gui\\quickslot_{0}.png", i + 1))));
		uiRenderer->SetSize(Vector2(105, 136));
		object->AddChild(m_slotBackground[i]);

		m_slotContents[i] = std::make_shared<SceneObject>();
		auto renderer = m_slotContents[i]->AddComponent<GUIImage>();
		m_slotContents[i]->GetTransform()->SetLocalPosition(Vector3(640.0f + 120.0f * i, -380.0f, 0.0f));
		renderer->SetSize(Vector2(100, 100));
		renderer->SetRaycastTarget(false);
		object->AddChild(m_slotContents[i]);

		m_slotText[i] = std::make_shared<SceneObject>();
		auto text = m_slotText[i]->AddComponent<GUIText>();
		text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		m_slotText[i]->GetTransform()->SetLocalPosition(Vector3(680.0f + 120.0f * i, -420.0f, 0.0f));
		text->SetRaycastTarget(false);
		object->AddChild(m_slotText[i]);
	}
}

void PlayerQuickSlotGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
    Component::Update(time, scene);
}

void PlayerQuickSlotGUI::UpdateSlotContents(const std::vector<int>& table, const std::vector<int>& tableInventory)
{
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (table[i] == -1)
		{
			m_slotContents[i]->GetComponent<GUIImage>()->SetTexture(nullptr);
			m_slotText[i]->GetComponent<GUIText>()->SetText(L"");
		}
		else
		{
			const std::string& key = DATA_TABLE->GetDropItemName(table[i]);
			udsdx::Texture* texture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, key, "Icon")));
			m_slotContents[i]->GetComponent<GUIImage>()->SetTexture(texture);
			m_slotText[i]->GetComponent<GUIText>()->SetText(std::to_wstring(tableInventory[table[i]]));
		}
	}
}
