#include "pch.h"
#include "PlayerInventoryGUI.h"
#include "AuthenticPlayer.h"

using namespace udsdx;

PlayerInventoryGUI::PlayerInventoryGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(640.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2(340.0f, 450.0f));
	object->AddChild(m_panel);

	for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
	{
		float x = (i % NUM_COLUMNS - (NUM_COLUMNS - 1) / 2.0f) * 110.0f;
		float y = (i / NUM_COLUMNS - (NUM_ROWS - 1) / 2.0f) * -110.0f;

		m_slotBackground[i] = std::make_shared<SceneObject>();
		auto uiRenderer = m_slotBackground[i]->AddComponent<GUIButton>();
		m_slotBackground[i]->GetTransform()->SetLocalPosition(Vector3(x, y, 0.0f));
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\item_slot.png")));
		uiRenderer->SetSize(Vector2(100, 100));
		m_panel->AddChild(m_slotBackground[i]);

		m_slotContents[i] = std::make_shared<SceneObject>();
		auto renderer = m_slotContents[i]->AddComponent<GUIImage>();
		m_slotContents[i]->GetTransform()->SetLocalPosition(Vector3(x, y, 0.0f));
		renderer->SetSize(Vector2(80, 80));
		renderer->SetRaycastTarget(false);
		m_panel->AddChild(m_slotContents[i]);

		m_slotText[i] = std::make_shared<SceneObject>();
		auto text = m_slotText[i]->AddComponent<GUIText>();
		text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		m_slotText[i]->GetTransform()->SetLocalPosition(Vector3(x + 40.0f, y - 40.0f, 0.0f));
		text->SetRaycastTarget(false);
		m_panel->AddChild(m_slotText[i]);
	}
}

void PlayerInventoryGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	Component::Update(time, scene);
}

void PlayerInventoryGUI::UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table)
{
	int counter = 0;

	const int item_count = static_cast<int>(DATA_TABLE->GetItemCount());
	for (int id = 0; id < item_count; id++)
	{
		if (table[id] > 0)
		{
			const std::string& key = DATA_TABLE->GetItemName(id);
			udsdx::Texture* texture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, key, "Icon")));
			m_slotBackground[counter]->GetComponent<GUIButton>()->SetClickCallback([target, id]() {
				target->SetQuickSlotItemOnBlank(id);
			});
			m_slotContents[counter]->GetComponent<GUIImage>()->SetTexture(texture);
			m_slotText[counter]->GetComponent<GUIText>()->SetText(std::to_wstring(table[id]));
			counter++;
		}
	}

	while (counter < NUM_ROWS * NUM_COLUMNS)
	{
		m_slotBackground[counter]->GetComponent<GUIButton>()->SetClickCallback(nullptr);
		m_slotContents[counter]->GetComponent<GUIImage>()->SetTexture(nullptr);
		m_slotText[counter]->GetComponent<GUIText>()->SetText(L"");
		counter++;
	}
}
