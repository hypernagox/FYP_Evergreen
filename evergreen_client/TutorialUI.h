#pragma once

class udsdx::GUIImage;

// 1. 움직이기
// 2. 공격
// 3. 스킬
// 4. 대쉬
// 5. 클리어 후 보상나무를 따라가서 보상 먹기
// 6. 제작대 가서 제작해보세요
// ----------------- 일단 튜토리얼 종료

// 7. K? 키를 눌러 가까운 채집물을 찾아서 채집
// 8. 파티게시판을 가보세요

enum class UI_TYPE
{
	WASD,
	ATTACK,

	SKILL,
	DASH,
	QUEST_1,
	CLEAR_TREE,
	CRAFT,
	INVENTORY,
	END_TUTORIAL_QUEST,

	NAVI_ITEM,
	NAVI_VILLAGE,
	PARTY,
	


	END,
};

class TutorialUIElementBase;

class TutorialUI
	: public udsdx::Component
{
public:
	virtual void OnInitialize() override;
	virtual void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
public:
	static void StartTutorialGUI();
public:
	static inline std::shared_ptr<TutorialUIElementBase> m_cur_gui;
	static inline bool m_waitFlag = false;
	static inline UI_TYPE m_nextType;
	static inline std::shared_ptr<TutorialUIElementBase> m_tutorialUIs[(int)UI_TYPE::END] = {};
	static inline std::shared_ptr<udsdx::SceneObject> m_tutorialMark;
	static inline std::unique_ptr<SoundEffectInstance> m_tickSound;
	static inline float m_accTime = 0.f;
	static constexpr const float TUTORIAL_UI_REMAIN_TIME = 3.f;
	static inline bool m_start_flag = false;
};


class TutorialUIElementBase
{
public:
	void Init(const std::shared_ptr<udsdx::SceneObject>& object,const UI_TYPE ui_type, const std::wstring_view path)noexcept;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene) = 0;
public:
	std::shared_ptr<udsdx::SceneObject> m_gui;
	UI_TYPE m_type;
	

	
};


class WASDTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene) override;

public:

	std::set<char> m_keyFlag;
};


class AttackTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};


class InventoryTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class NaviItemTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class PartyTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
	bool m_flag = false;
	float m_accTime = 0.f;
};


class NaviVillageTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;

	float m_accTime = 0.f;
	bool m_flag = false;
};

class QuestTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

// 추가

class SkillTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class DashTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class ClearTreeTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
	int m_e_count = 3;
};

class CraftTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class EndTutorialQuestTutorial
	:public TutorialUIElementBase
{
public:
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};
