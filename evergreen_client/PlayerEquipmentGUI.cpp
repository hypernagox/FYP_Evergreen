#include "pch.h"
#include "PlayerEquipmentGUI.h"
#include "AuthenticPlayer.h"
#include "PopupGUIManager.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void PlayerEquipmentGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	m_panel->GetTransform()->SetLocalPosition(Vector3(640.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\equipment.png")), true);
	uiRenderer->SetSize(Vector2(452.0f, 600.0f));
	GetSceneObject()->AddChild(m_panel);

	m_statText = SceneObject::MakeShared();
	auto text = m_statText->AddComponent<GUIText>();
	text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	text->SetAlignment(GUIText::Alignment::UpperLeft);
	m_statText->GetTransform()->SetLocalPosition(Vector3(-186.0f, -27.0f, 0.0f));
	text->SetRaycastTarget(false);
	m_panel->AddChild(m_statText);

	for (int i = 0; i < 2; i++)
	{
		float y = 115.0f - i * 78.0f;

		m_slotContents[i] = SceneObject::MakeShared();
		auto renderer = m_slotContents[i]->AddComponent<GUISimpleButton>();
		m_slotContents[i]->GetTransform()->SetLocalPosition(Vector3(-162.0f, y, 0.0f));
		renderer->SetSize(Vector2(60, 60));
		renderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\icon_priestequipment_1.png")));
		m_panel->AddChild(m_slotContents[i]);

		m_slotText[i] = SceneObject::MakeShared();
		auto text = m_slotText[i]->AddComponent<GUIText>();
		text->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		text->SetAlignment(GUIText::Alignment::Left);
		m_slotText[i]->GetTransform()->SetLocalPosition(Vector3(-130.0f, y + 12.0f, 0.0f));
		text->SetRaycastTarget(false);
		m_panel->AddChild(m_slotText[i]);
	}

	auto exitButton = SceneObject::MakeShared();
	auto exitButtonRenderer = exitButton->AddComponent<GUISimpleButton>();
	exitButton->GetTransform()->SetLocalPosition(Vector3(175.0f, 250.0f, 0.0f));
	exitButtonRenderer->SetSize(Vector2(50.0f, 50.0f));
	exitButtonRenderer->SetClickCallback([&]() {
		GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
		});
	m_panel->AddChild(exitButton);

	UpdateSlotContents(nullptr, 0, -1);
	UpdateSlotContents(nullptr, 1, -1);
}

void PlayerEquipmentGUI::UpdateSlotContents(AuthenticPlayer* target, int index, int id)
{
	itemIDCache[index] = id;

	auto slotContents = m_slotContents[index]->GetComponent<GUISimpleButton>();
	auto slotText = m_slotText[index]->GetComponent<GUIText>();

	m_slotContents[index]->SetActive(id >= 0);
	if (id < 0)
	{
		slotText->SetText(index > 0 ? L"장비 없음" : L"무기 없음");
	}
	else
	{
		const std::string& key = DATA_TABLE->GetItemName(id);
		udsdx::Texture* texture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, "Item", key, "Icon")));
		std::wstring itemName = GET_DATA(std::wstring, "Item", key, "Name");
		slotContents->SetTexture(texture);
		slotText->SetText(itemName);
	}

	std::wstring statTextContent;
	if (itemIDCache[0] >= 0)
	{
		const std::string& itemName = DATA_TABLE->GetItemName(itemIDCache[0]);
		auto weaponKey = GET_DATA(std::string, "Item", itemName, "WeaponKey");
		int weaponID = DATA_TABLE->GetWeaponIDInt(weaponKey);
		auto equipmentStat = DATA_TABLE->GetEquipStat(weaponID);
		statTextContent += L"공격력: +" + std::to_wstring(equipmentStat.atk) + L"\n";
	}
	if (itemIDCache[1] >= 0)
	{
		statTextContent += L"방어력: +1\n";
	}
	m_statText->GetComponent<GUIText>()->SetText(statTextContent);
}
