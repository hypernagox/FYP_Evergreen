#include "pch.h"
#include "PlayerQuickSlotGUI.h"

using namespace udsdx;

PlayerQuickSlotGUI::PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
    for (int i = 0; i < NUM_SLOTS; i++)
    {
		m_slotBackground[i] = std::make_shared<SceneObject>();
		auto uiRenderer = m_slotBackground[i]->AddComponent<GUIImage>();
		m_slotBackground[i]->GetTransform()->SetLocalPosition(Vector3(640.0f + 120.0f * i, -400.0f, 0.0f));
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(std::format(L"gui\\quickslot_{0}.png", i + 1))));
		uiRenderer->SetSize(Vector2Int(105, 136));
		object->AddChild(m_slotBackground[i]);

		m_slotContents[i] = std::make_shared<SceneObject>();
		auto renderer = m_slotContents[i]->AddComponent<GUIImage>();
		m_slotContents[i]->GetTransform()->SetLocalPosition(Vector3(640.0f + 120.0f * i, -380.0f, 0.0f));
		renderer->SetSize(Vector2Int(100, 100));
		object->AddChild(m_slotContents[i]);
	}
}

void PlayerQuickSlotGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
    Component::Update(time, scene);
}

void PlayerQuickSlotGUI::SetSlotContents(int slotIndex, uint8_t item_id)
{
	const std::string& key = DATA_TABLE->GetDropItemName(item_id);
	udsdx::Texture* texture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, key, "Icon")));
	m_slotContents[slotIndex]->GetComponent<GUIImage>()->SetTexture(texture);
}
