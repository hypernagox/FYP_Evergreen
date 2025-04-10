#include "pch.h"
#include "PlayerCraftGUI.h"
#include "AuthenticPlayer.h"

using namespace udsdx;

PlayerCraftGUI::PlayerCraftGUI(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(-640.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2(480.0f, 640.0f));
	object->AddChild(m_panel);

	// TODO: 1 is a magic number
	for (int i = 0; i < 1; i++)
	{
		const auto& combine_list = GET_RECIPE("C");

		float y = i * -130.0f + 250.0f;
		auto& recipeGUI = m_recipePanels.emplace_back();
		recipeGUI.Panel = std::make_shared<SceneObject>();
		recipeGUI.Panel->GetTransform()->SetLocalPositionY(y);
		auto panelRenderer = recipeGUI.Panel->AddComponent<GUIImage>();
		panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
		panelRenderer->SetSize(Vector2(460.0f, 120.0f));
		m_panel->AddChild(recipeGUI.Panel);

		recipeGUI.OutputSlotBackground = std::make_shared<SceneObject>();
		auto outputBackgroundRenderer = recipeGUI.OutputSlotBackground->AddComponent<GUIImage>();
		recipeGUI.OutputSlotBackground->GetTransform()->SetLocalPosition(Vector3(-160.0f, 0.0f, 0.0f));
		outputBackgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\item_slot.png")));
		outputBackgroundRenderer->SetSize(Vector2(120, 120));
		recipeGUI.Panel->AddChild(recipeGUI.OutputSlotBackground);

		recipeGUI.OutputSlotContents = std::make_shared<SceneObject>();
		auto outputContentsRenderer = recipeGUI.OutputSlotContents->AddComponent<GUIImage>();
		recipeGUI.OutputSlotContents->GetTransform()->SetLocalPosition(Vector3(-160.0f, 0.0f, 0.0f));
		// TODO: Dynamic texture loading
		outputContentsRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\itemicon_coin.png")));
		outputContentsRenderer->SetSize(Vector2(100, 100));
		outputContentsRenderer->SetRaycastTarget(false);
		recipeGUI.Panel->AddChild(recipeGUI.OutputSlotContents);

		recipeGUI.OutputSlotText = std::make_shared<SceneObject>();
		auto outputText = recipeGUI.OutputSlotText->AddComponent<GUIText>();
		outputText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		recipeGUI.OutputSlotText->GetTransform()->SetLocalPosition(Vector3(-120.0f, -40.0f, 0.0f));
		outputText->SetRaycastTarget(false);
		outputText->SetText(L"1");
		recipeGUI.Panel->AddChild(recipeGUI.OutputSlotText);

		recipeGUI.CraftButton = std::make_shared<SceneObject>();
		auto craftButtonRenderer = recipeGUI.CraftButton->AddComponent<GUIButton>();
		recipeGUI.CraftButton->GetTransform()->SetLocalPosition(Vector3(160.0f, 0.0f, 0.0f));
		craftButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
		craftButtonRenderer->SetSize(Vector2(100, 100));
		craftButtonRenderer->SetInteractable(false);
		recipeGUI.Panel->AddChild(recipeGUI.CraftButton);

		int j = 0;
		for (const auto& combine : combine_list)
		{
			float x = j * 65.0f - 65.0f;
			auto iconPath = GET_DATA(std::wstring, combine.itemName, "Icon");

			auto inputSlotBackground = recipeGUI.InputSlotBackground.emplace_back(std::make_shared<SceneObject>());
			auto inputBackgroundRenderer = inputSlotBackground->AddComponent<GUIImage>();
			inputSlotBackground->GetTransform()->SetLocalPosition(Vector3(x, -25.0f, 0.0f));
			inputBackgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\item_slot.png")));
			inputBackgroundRenderer->SetSize(Vector2(60, 60));
			recipeGUI.Panel->AddChild(inputSlotBackground);

			auto inputSlotContents = recipeGUI.InputSlotContents.emplace_back(std::make_shared<SceneObject>());
			auto inputContentsRenderer = inputSlotContents->AddComponent<GUIImage>();
			inputSlotContents->GetTransform()->SetLocalPosition(Vector3(x, -25.0f, 0.0f));
			inputContentsRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(iconPath)));
			inputContentsRenderer->SetSize(Vector2(50, 50));
			inputContentsRenderer->SetRaycastTarget(false);
			recipeGUI.Panel->AddChild(inputSlotContents);

			auto inputSlotText = recipeGUI.InputSlotText.emplace_back(std::make_shared<SceneObject>());
			auto inputText = inputSlotText->AddComponent<GUIText>();
			inputText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
			inputSlotText->GetTransform()->SetLocalPosition(Vector3(x + 20.0f, -45.0f, 0.0f));
			inputText->SetRaycastTarget(false);
			inputText->SetText(std::to_wstring(combine.numOfRequire));
			recipeGUI.Panel->AddChild(inputSlotText);

			++j;
		}
	}
}

void PlayerCraftGUI::UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table)
{
	for (int i = 0; i < m_recipePanels.size(); i++)
	{
		const auto& combine_list = GET_RECIPE("C");
		bool available = true;
		for (const auto& [itemName, numOfRequire] : combine_list)
		{
			const auto item_id = DATA_TABLE->GetDropItemID(itemName);
			const auto diff = table[item_id] - numOfRequire;
			if (diff < 0)
			{
				available = false;
				break;
			}
		}

		auto buttonComponent = m_recipePanels[i].CraftButton->GetComponent<GUIButton>();
		buttonComponent->SetInteractable(available);
		buttonComponent->SetClickCallback([target]() {
			// TODO: need to specity the recipe ID
			target->CraftItem(0);
			});
	}
}
