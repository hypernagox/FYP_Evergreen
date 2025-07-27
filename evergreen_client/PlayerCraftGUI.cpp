#include "pch.h"
#include "PlayerCraftGUI.h"
#include "AuthenticPlayer.h"
#include "GUISimpleButton.h"
#include "PopupGUIManager.h"

using namespace udsdx;

void PlayerCraftGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	m_panel->GetTransform()->SetLocalPosition(Vector3(-640.0f, 0.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\craft.png")), true);
	GetSceneObject()->AddChild(m_panel);

	const int recipe_count = static_cast<int>(DATA_TABLE->GetRecipeCount());
	for (int id = 0; id < recipe_count; id++)
	{
		const auto& combine_list = DATA_TABLE->GetItemRecipe(id);
		auto iconPath = GET_DATA(std::wstring, "Item", combine_list.resultItem, "Icon");

		float x = -170.0f + (id % 2) * 212.0f;
		float y = 125.0f - (id / 2) * 89.0f;

		auto& recipeGUI = m_recipePanels.emplace_back();
		recipeGUI.Panel = SceneObject::MakeShared();
		recipeGUI.Panel->GetTransform()->SetLocalPosition(Vector3(x, y, 0.0f));
		m_panel->AddChild(recipeGUI.Panel);

		recipeGUI.OutputSlotContents = SceneObject::MakeShared();
		auto outputContentsRenderer = recipeGUI.OutputSlotContents->AddComponent<GUIImage>();
		outputContentsRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(iconPath)));
		outputContentsRenderer->SetSize(Vector2(64, 64));
		outputContentsRenderer->SetRaycastTarget(false);
		recipeGUI.Panel->AddChild(recipeGUI.OutputSlotContents);

		recipeGUI.CraftButton = SceneObject::MakeShared();
		auto craftButtonRenderer = recipeGUI.CraftButton->AddComponent<GUISimpleButton>();
		recipeGUI.CraftButton->GetTransform()->SetLocalPosition(Vector3(133.0f, 26.0f, 0.0f));
		craftButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
		craftButtonRenderer->SetSize(Vector2(60, 16));
		craftButtonRenderer->SetInteractable(false);
		recipeGUI.Panel->AddChild(recipeGUI.CraftButton);

		recipeGUI.CraftButtonText = SceneObject::MakeShared();
		auto craftButtonText = recipeGUI.CraftButtonText->AddComponent<GUIText>();
		craftButtonText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		craftButtonText->SetRaycastTarget(false);
		craftButtonText->SetText(L"재료 부족");
		craftButtonText->GetTransform()->SetLocalScale(0.6f);
		craftButtonText->SetAlignment(GUIText::Alignment::Center);
		recipeGUI.CraftButton->AddChild(recipeGUI.CraftButtonText);

		recipeGUI.OutputNameText = SceneObject::MakeShared();
		auto outputNameText = recipeGUI.OutputNameText->AddComponent<GUIText>();
		outputNameText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
		recipeGUI.OutputNameText->GetTransform()->SetLocalPosition(Vector3(44.0f, -23.0f, 0.0f));
		recipeGUI.OutputNameText->GetTransform()->SetLocalScale(0.75f);
		outputNameText->SetAlignment(GUIText::Alignment::Left);
		outputNameText->SetRaycastTarget(false);
		outputNameText->SetText(GET_DATA(std::wstring, "Item", combine_list.resultItem, "Name"));
		outputNameText->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		recipeGUI.Panel->AddChild(recipeGUI.OutputNameText);

		int j = 0;
		for (const auto& combine : combine_list.itemElements)
		{
			float x = j * 25.0f + 52.0f;
			auto iconPath = GET_DATA(std::wstring, "Item", combine.itemName, "Icon");

			auto inputSlotBackground = recipeGUI.InputSlotBackground.emplace_back(SceneObject::MakeShared());
			auto inputBackgroundRenderer = inputSlotBackground->AddComponent<GUISimpleButton>();
			inputSlotBackground->GetTransform()->SetLocalPosition(Vector3(x, 5.0f, 0.0f));
			inputBackgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\item_slot.png")));
			inputBackgroundRenderer->SetSize(Vector2(25, 25));
			recipeGUI.Panel->AddChild(inputSlotBackground);

			auto inputSlotContents = recipeGUI.InputSlotContents.emplace_back(SceneObject::MakeShared());
			auto inputContentsRenderer = inputSlotContents->AddComponent<GUIImage>();
			inputSlotContents->GetTransform()->SetLocalPosition(Vector3(x, 5.0f, 0.0f));
			inputContentsRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(iconPath)));
			inputContentsRenderer->SetSize(Vector2(25, 25));
			inputContentsRenderer->SetRaycastTarget(false);
			recipeGUI.Panel->AddChild(inputSlotContents);

			auto inputSlotText = recipeGUI.InputSlotText.emplace_back(SceneObject::MakeShared());
			auto inputText = inputSlotText->AddComponent<GUIText>();
			inputText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
			inputSlotText->GetTransform()->SetLocalPosition(Vector3(x + 10.0f, -7.5f, 0.0f));
			inputSlotText->GetTransform()->SetLocalScale(0.5f);
			inputText->SetAlignment(GUIText::Alignment::LowerRight);
			inputText->SetRaycastTarget(false);
			inputText->SetText(std::to_wstring(combine.numOfRequire));
			recipeGUI.Panel->AddChild(inputSlotText);

			++j;
		}

		auto exitButton = SceneObject::MakeShared();
		auto exitButtonRenderer = exitButton->AddComponent<GUISimpleButton>();
		exitButton->GetTransform()->SetLocalPosition(Vector3(175.0f, 250.0f, 0.0f));
		exitButtonRenderer->SetSize(Vector2(50.0f, 50.0f));
		exitButtonRenderer->SetClickCallback([&]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
			});
		m_panel->AddChild(exitButton);
	}
}

void PlayerCraftGUI::OnActive()
{
	m_panel->GetTransform()->SetLocalPositionY(-50.0f);
}

void PlayerCraftGUI::Update(const Time& time, Scene& scene)
{
	m_panel->GetTransform()->SetLocalPositionY(std::lerp(m_panel->GetTransform()->GetLocalPosition().y, 0.0f, time.deltaTime * 16.0f));
}

void PlayerCraftGUI::UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table)
{
	// 골라진 레시피 아이디
	int recipe_id = -1;
	for (size_t i = 0; i < m_recipePanels.size(); i++)
	{
		const auto& combine_list = GET_RECIPE(static_cast<int>(i));
		recipe_id = combine_list.recipeID;

		// 해당 레시피가 재료의 개수를 충족하는지 확인
		// available 이 true면 재료가 충분하다는 뜻
		// 각각의 재료에 대해서 table(인벤토리)의 개수와 비교
		bool available = true;
		for (const auto& [itemName,itemId, numOfRequire] : combine_list.itemElements)
		{
			const auto item_id = DATA_TABLE->GetItemID(itemName);
			const auto diff = table[item_id] - numOfRequire;
			if (diff < 0)
			{
				available = false;
				break;
			}
		}

		auto buttonComponent = m_recipePanels[i].CraftButton->GetComponent<GUISimpleButton>();
		auto buttonText = m_recipePanels[i].CraftButtonText->GetComponent<GUIText>();

		// 조건이 충족될 경우 버튼을 활성화하고 클릭 시 레시피를 제작하는 콜백을 설정
		buttonComponent->SetInteractable(available);
		if (available)
		{
			buttonComponent->SetClickCallback([recipe_id, target]() {
				target->CraftItem(recipe_id);
				});
		}
		buttonText->SetText(available ? L"제작" : L"재료 부족");
	}
}
