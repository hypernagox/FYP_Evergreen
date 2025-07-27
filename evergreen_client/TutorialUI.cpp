#include "pch.h"
#include "TutorialUI.h"
#include "scene_object.h"
#include "gui_image.h"
#include "input.h"
#include "GuideSystem.h"

using namespace udsdx;

bool g_tutorial_clear = false;
bool g_tutorial_craft_clear = false;
bool g_tutorial_end_clear = false;
bool g_tutorial_navi_item = false;
bool g_tutorial_find_quest = false;

void TutorialUI::OnInitialize()
{
	auto object = GetSceneObject();

	{
		// WASD
		auto& gui = m_tutorialUIs[(int)UI_TYPE::WASD];
		gui = std::make_shared<WASDTutorial>();
		gui->Init(object,UI_TYPE::WASD, L"gui\\tutorial\\WASD.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// ATTACK
		auto& gui = m_tutorialUIs[(int)UI_TYPE::ATTACK];
		gui = std::make_shared<AttackTutorial>();
		gui->Init(object,UI_TYPE::ATTACK, L"gui\\tutorial\\Attack.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// SKILL
		auto& gui = m_tutorialUIs[(int)UI_TYPE::SKILL];
		gui = std::make_shared<SkillTutorial>();
		// TODO: 리소스 및 경로
		gui->Init(object, UI_TYPE::SKILL, L"gui\\tutorial\\right_skill.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// DASH
		auto& gui = m_tutorialUIs[(int)UI_TYPE::DASH];
		gui = std::make_shared<DashTutorial>();
		// TODO: 리소스 및 경로
		gui->Init(object, UI_TYPE::DASH, L"gui\\tutorial\\space_dash.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// QUEST_1
		auto& gui = m_tutorialUIs[(int)UI_TYPE::QUEST_1];
		gui = std::make_shared<QuestTutorial>();
		gui->Init(object, UI_TYPE::QUEST_1, L"gui\\tutorial\\npc_guard.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// CLEAR_TREE
		auto& gui = m_tutorialUIs[(int)UI_TYPE::CLEAR_TREE];
		gui = std::make_shared<ClearTreeTutorial>();
		// TODO: 리소스 및 경로
		gui->Init(object, UI_TYPE::CLEAR_TREE, L"gui\\tutorial\\clear_tree.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// CRAFT
		auto& gui = m_tutorialUIs[(int)UI_TYPE::CRAFT];
		gui = std::make_shared<CraftTutorial>();
		// TODO: 리소스 및 경로
		gui->Init(object, UI_TYPE::CRAFT, L"gui\\tutorial\\craft_item.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// INVENTORY
		auto& gui = m_tutorialUIs[(int)UI_TYPE::INVENTORY];
		gui = std::make_shared<InventoryTutorial>();
		gui->Init(object, UI_TYPE::INVENTORY, L"gui\\tutorial\\inventory_tutorial.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		// END_TUTORIAL
		auto& gui = m_tutorialUIs[(int)UI_TYPE::END_TUTORIAL_QUEST];
		gui = std::make_shared<EndTutorialQuestTutorial>();
		// TODO 새 리소스 및 경로
		gui->Init(object, UI_TYPE::END_TUTORIAL_QUEST, L"gui\\tutorial\\tutorial_out.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	// ----------------------- 튜토리얼은 끝 --------------------------
	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::NAVI_ITEM];
		gui = std::make_shared<NaviItemTutorial>();
		gui->Init(object, UI_TYPE::NAVI_ITEM, L"gui\\tutorial\\harvest_navi.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::NAVI_VILLAGE];
		gui = std::make_shared<NaviVillageTutorial>();
		gui->Init(object, UI_TYPE::NAVI_VILLAGE, L"gui\\tutorial\\find_quest.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	{
		auto& gui = m_tutorialUIs[(int)UI_TYPE::PARTY];
		gui = std::make_shared<PartyTutorial>();
		gui->Init(object, UI_TYPE::PARTY, L"gui\\tutorial\\create_quest.png");
		gui->m_gui->SetActive(false);
		object->AddChild(gui->m_gui);
	}

	m_tutorialUIs[(int)UI_TYPE::WASD]->m_gui->SetActive(false);
	m_cur_gui = m_tutorialUIs[(int)UI_TYPE::WASD];

	m_tutorialMark = SceneObject::MakeShared();
	m_tutorialMark->GetTransform()->SetLocalPosition(Vector3(0, 630.0f, 0.0f));

	auto uiRenderer = m_tutorialMark->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\tutorial\\qmark.png")), true);
	uiRenderer->SetSize(uiRenderer->GetSize() * 0.4f);
	m_tutorialMark->SetActive(false);
	object->AddChild(m_tutorialMark);
}

void TutorialUIElementBase::Init(const std::shared_ptr<udsdx::SceneObject>& object, const UI_TYPE ui_type, const std::wstring_view path) noexcept
{
	m_gui = SceneObject::MakeShared();
	m_gui->GetTransform()->SetLocalPosition(Vector3(0, 500.0f, 0.0f));
	m_type = ui_type;

	auto uiRenderer = m_gui->AddComponent<GUIImage>();

	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(path)), true);
	uiRenderer->SetSize(uiRenderer->GetSize() * 0.4f);
}


void TutorialUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (!m_start_flag)
	{
		return;
	}
	if (m_cur_gui)
	{
		if (!m_waitFlag)
		{
			const auto prev_type = m_cur_gui->m_type;
			const auto cur_type = m_cur_gui->Update(time, scene);
			if (UI_TYPE::END == cur_type)
			{
				m_cur_gui->m_gui->SetActive(false);
				m_tutorialMark->SetActive(false);
				m_start_flag = false;
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
				m_tutorialMark->SetActive(true);
				// TODO: 여기가 튜토리얼 넘어가는시점

				m_tickSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\tutorial_tick.wav"))->CreateInstance();
				m_tickSound->Play();
			}
		}
	}
}

void TutorialUI::StartTutorialGUI()
{
	m_start_flag = true;
	m_tutorialUIs[(int)UI_TYPE::WASD]->m_gui->SetActive(true);
	m_tutorialMark->SetActive(true);
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
	
	if (INSTANCE(Input)->GetKeyDown(Keyboard::D))
	{
		m_keyFlag.emplace('D');
	}

	const auto n = m_keyFlag.size();
	if (n == 3)
	{
		m_keyFlag.clear();
		return UI_TYPE::ATTACK;
	}
	else
	{
		return m_type;
	}
}

UI_TYPE InventoryTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{	
	if (INSTANCE(Input)->GetKeyDown(Keyboard::I))
	{
		return UI_TYPE::END_TUTORIAL_QUEST;
	}
	return m_type;
}

UI_TYPE NaviItemTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_tutorial_navi_item)
	{
		g_tutorial_navi_item = false;
		return UI_TYPE::NAVI_VILLAGE;
	}
	return m_type;
}


UI_TYPE NaviVillageTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_tutorial_find_quest)
	{
		g_tutorial_find_quest = false;
		return UI_TYPE::PARTY;
	}
	return m_type;
}

UI_TYPE QuestTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_tutorial_clear)
	{
		g_tutorial_clear = false;
		return UI_TYPE::CLEAR_TREE;
	}
	return m_type;
}

UI_TYPE PartyTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	
	if (INSTANCE(Input)->GetKeyDown(Keyboard::E))
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

UI_TYPE AttackTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetMouseLeftButtonDown())
	{
		return UI_TYPE::SKILL;
	}
	else return m_type;
}

UI_TYPE SkillTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetMouseRightButtonDown())
	{
		return UI_TYPE::DASH;
	}
	return m_type;
}

UI_TYPE DashTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::Space))
	{
		return UI_TYPE::QUEST_1;
	}
	return m_type;
}

UI_TYPE ClearTreeTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::E))
	{
		--m_e_count;
		if (0 == m_e_count)
		{
			GuideSystem::GetInst()->ToggleFlag();
			GuideSystem::GetInst()->temp_force_pos = Vector3(-123.62704F, 75.79371F, 15.156882F);
			return UI_TYPE::CRAFT;
		}
	}
	return m_type;
}

UI_TYPE CraftTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_tutorial_craft_clear)
	{
		g_tutorial_craft_clear = false;
		return UI_TYPE::INVENTORY;
	}
	return m_type;
}

UI_TYPE EndTutorialQuestTutorial::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (g_tutorial_end_clear)
	{
		g_tutorial_end_clear = false;
		return UI_TYPE::NAVI_ITEM;
	}
	return m_type;
}
