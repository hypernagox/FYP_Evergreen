#pragma once

class udsdx::GUIImage;

enum class UI_TYPE
{
	WASD,
	ATTACK,
	INVENTORY,
	NAVI_ITEM,
	NAVI_VILLAGE,
	PARTY,
	QUEST_1,


	END,
};

class TutorialUIElementBase;

class TutorialUI
	: public udsdx::Component
{
public:
	TutorialUI(const std::shared_ptr<udsdx::SceneObject>& object);


public:
	virtual void Update(const udsdx::Time& time, udsdx::Scene& scene)override;

public:
	std::shared_ptr<TutorialUIElementBase> m_cur_gui;
	bool m_waitFlag = false;
	UI_TYPE m_nextType;
	std::shared_ptr<TutorialUIElementBase> m_tutorialUIs[(int)UI_TYPE::END] = {};
	std::shared_ptr<udsdx::SceneObject> m_tutorialMark;
	std::unique_ptr<SoundEffectInstance> m_tickSound;

	float m_accTime = 0.f;

	static constexpr const float TUTORIAL_UI_REMAIN_TIME = 3.f;
};


class TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) = 0;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene) = 0;
public:
	std::shared_ptr<udsdx::SceneObject> m_gui;
	UI_TYPE m_type;
	

	
};


class WASDTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;

public:

	std::set<char> m_keyFlag;
};


class AttackTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};


class InventoryTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class NaviItemTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};

class PartyTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};


class NaviVillageTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;

	float m_accTime = 0.f;
	bool m_flag = false;
};

class QuestTutorial
	:public TutorialUIElementBase
{
public:
	virtual void Init(const std::shared_ptr<udsdx::SceneObject>& object) override;
	virtual UI_TYPE Update(const udsdx::Time& time, udsdx::Scene& scene)override;
};