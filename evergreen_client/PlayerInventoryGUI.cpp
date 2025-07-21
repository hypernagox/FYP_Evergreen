#include "pch.h"
#include "PlayerInventoryGUI.h"
#include "AuthenticPlayer.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void PlayerInventoryGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	m_panel->GetTransform()->SetLocalPosition(Vector3(640.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\inventory.png")));
	uiRenderer->SetSize(Vector2(360.0f, 520.0f));
	GetSceneObject()->AddChild(m_panel);

	for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
	{
		float x = (i % NUM_COLUMNS - (NUM_COLUMNS - 1) / 2.0f) * 81.0f;
		float y = (i / NUM_COLUMNS - (NUM_ROWS - 1) / 2.0f) * -81.0f + 8.0f;

		m_slotBackground[i] = SceneObject::MakeShared();
		auto uiRenderer = m_slotBackground[i]->AddComponent<GUISimpleButton>();
		m_slotBackground[i]->GetTransform()->SetLocalPosition(Vector3(x, y, 0.0f));
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\item_slot.png")));
		uiRenderer->SetSize(Vector2(80, 80));
		m_panel->AddChild(m_slotBackground[i]);

		m_slotContents[i] = SceneObject::MakeShared();
		auto renderer = m_slotContents[i]->AddComponent<GUIImage>();
		m_slotContents[i]->GetTransform()->SetLocalPosition(Vector3(x, y, 0.0f));
		renderer->SetSize(Vector2(60, 60));
		renderer->SetRaycastTarget(false);
		m_panel->AddChild(m_slotContents[i]);

		m_slotText[i] = SceneObject::MakeShared();
		auto text = m_slotText[i]->AddComponent<GUIText>();
		text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		m_slotText[i]->GetTransform()->SetLocalPosition(Vector3(x + 25.0f, y - 25.0f, 0.0f));
		text->SetRaycastTarget(false);
		m_panel->AddChild(m_slotText[i]);
	}

	m_coinText = SceneObject::MakeShared();
	m_coinText->GetTransform()->SetLocalPosition(Vector3(150.0f, -224.0f, 0.0f));
	auto text = m_coinText->AddComponent<GUIText>();
	text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	text->SetRaycastTarget(false);
	text->SetText(L"000 000 000");
	text->SetAlignment(GUIText::Alignment::Right);
	m_panel->AddChild(m_coinText);
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
			udsdx::Texture* texture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring,"Item", key, "Icon")));
			m_slotBackground[counter]->GetComponent<GUISimpleButton>()->SetClickCallback([this, target, id]() {
				SelectInventorySlot(target, id);
			});
			m_slotContents[counter]->GetComponent<GUIImage>()->SetTexture(texture);
			m_slotText[counter]->GetComponent<GUIText>()->SetText(std::to_wstring(table[id]));
			counter++;
		}
		if (counter >= NUM_ROWS * NUM_COLUMNS)
			break;
	}

	while (counter < NUM_ROWS * NUM_COLUMNS)
	{
		m_slotBackground[counter]->GetComponent<GUISimpleButton>()->SetClickCallback(nullptr);
		m_slotContents[counter]->GetComponent<GUIImage>()->SetTexture(nullptr);
		m_slotText[counter]->GetComponent<GUIText>()->SetText(L"");
		counter++;
	}
}

void PlayerInventoryGUI::SelectInventorySlot(AuthenticPlayer* target, int id)
{
	auto itemName = DATA_TABLE->GetItemName(id);
	auto category = GET_DATA(std::string,"Item", itemName, "Category");
	if (category == "Consumable")
		target->SetQuickSlotItemOnBlank(id);
	else if (category == "Equipment")
	{
		const auto subcategory = GET_DATA(std::string, "Item", itemName, "Subcategory");

		m_equipSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\equip.wav"))->CreateInstance();
		m_equipSound->SetVolume(0.5f);
		m_equipSound->Play();

		if (subcategory == "Weapon")
		{
			auto weaponID = DATA_TABLE->GetWeaponIDInt(GET_DATA(std::string, "Item", itemName, "WeaponKey"));
			target->SetPlayerWeapon(weaponID, true);
		}
		else if (subcategory == "Armor")
		{
			auto armorID = DATA_TABLE->GetArmorIDInt(GET_DATA(std::string, "Item", itemName, "ArmorKey"));
			target->SetPlayerArmor(armorID, true);
		}
	}
}
