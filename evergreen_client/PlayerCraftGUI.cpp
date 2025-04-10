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

	for (int i = 0; i < 10; i++)
	{
		const auto& combine_list = GET_RECIPE("C");

		float y = i * -210.0f;
		auto& recipeGUI = m_recipePanels.emplace_back();
		recipeGUI.Panel = std::make_shared<SceneObject>();
		recipeGUI.Panel->GetTransform()->SetLocalPositionY(y);
		auto panelRenderer = recipeGUI.Panel->AddComponent<GUIImage>();
		panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
		panelRenderer->SetSize(Vector2(460.0f, 160.0f));
		m_panel->AddChild(recipeGUI.Panel);
	}
}

void PlayerCraftGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
}

void PlayerCraftGUI::UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table)
{
}
