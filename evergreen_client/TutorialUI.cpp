#include "pch.h"
#include "TutorialUI.h"
#include "scene_object.h"
#include "gui_image.h"
#include "input.h"

using namespace udsdx;

bool g_first_clear = false;

TutorialUI::TutorialUI(const std::shared_ptr<udsdx::SceneObject>& object)
	:Component{ object }
{

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::WASD];
		gui = std::make_shared<WASDTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::ATTACK];
		gui = std::make_shared<AttackTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::INVENTORY];
		gui = std::make_shared<InventoryTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::NAVI_ITEM];
		gui = std::make_shared<NaviItemTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::NAVI_VILLAGE];
		gui = std::make_shared<NaviVillageTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::PARTY];
		gui = std::make_shared<PartyTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::QUEST_1];
		gui = std::make_shared<QuestTutorial>();
		gui->Init(object);
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	m_tutorialUIs[(int)UI_TYPE::WASD]->m_gui->SetActive(true);
	m_cur_gui = m_tutorialUIs[(int)UI_TYPE::WASD];
}

void TutorialUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (m_cur_gui)
	{
		if (!m_waitFlag)
		{
			const auto prev_type = m_cur_gui->m_type;
			const auto cur_type = m_cur_gui->Update(time, scene);
			if (UI_TYPE::END == cur_type)
			{
				m_cur_gui->m_gui->SetActive(false);
				return;
			}
			if (prev_type != cur_type && !m_waitFlag)
			{
				// TODO: 여기가 튜토리얼 클리어시점
				m_nextType = cur_type;
				m_waitFlag = true;
			}
		}
		else
		{
			m_accTime += DT;
			if (TUTORIAL_UI_REMAIN_TIME <= m_accTime)
			{
				m_accTime = 0.f;
				m_waitFlag = false;
				m_cur_gui->m_gui->SetActive(false);
				m_cur_gui = m_tutorialUIs[(int)m_nextType];
				m_cur_gui->m_gui->SetActive(true);
				// TODO: 여기가 튜토리얼 넘어가는시점
			
			}
		}
	}
}

void WASDTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::WASD;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\WASD.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE WASDTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::W))
	{
		m_keyFlag.emplace('W');
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::A))
	{
		m_keyFlag.emplace('A');
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::S))
	{
		m_keyFlag.emplace('S');
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::D))
	{
		m_keyFlag.emplace('D');
	}

	const auto n = m_keyFlag.size();
	if (n == 4)
	{
		m_keyFlag.clear();
		return UI_TYPE::ATTACK;
	}
	else
	{
		return m_type;
	}
}

void InventoryTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::INVENTORY;
	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Inventory.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE InventoryTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{	
	if (INSTANCE(Input)->GetKeyDown(Keyboard::I))
	{
		return UI_TYPE::NAVI_VILLAGE;
	}
	return m_type;
}

void NaviItemTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::NAVI_ITEM;
	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Navi_item.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE NaviItemTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::P))
	{
		return UI_TYPE::INVENTORY;
	}
	return m_type;
}

void NaviVillageTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::NAVI_VILLAGE;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Navi_village.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}


UI_TYPE NaviVillageTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::K))
	{
		m_flag = true;
	}
	if (m_flag)
	{
		m_accTime += DT;
		if (TutorialUI::TUTORIAL_UI_REMAIN_TIME <= m_accTime)
		{
			return UI_TYPE::END;
		}
	}
	return m_type;
}

void QuestTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::QUEST_1;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Quest_1.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE QuestTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_first_clear)
	{
		return UI_TYPE::NAVI_ITEM;
	}
	return m_type;
}

void PartyTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::PARTY;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Party.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE PartyTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::Q))
	{
		return UI_TYPE::QUEST_1;
	}
	return m_type;
}

void AttackTutorial::Init(const std::shared_ptr<udsdx::SceneObject>& object)
{
	m_gui = std::make_shared<SceneObject>();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 360.0f, 0.0f));
	m_type = UI_TYPE::ATTACK;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\Attack.png")));

	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
}

UI_TYPE AttackTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetMouseLeftButtonDown())
	{
		return UI_TYPE::PARTY;
	}
	else return m_type;
}
