#pragma once
#include "ContentsComponent.h"

class Quest;

class QuestSystem
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(QuestSystem)
public:
	virtual ~QuestSystem()noexcept;
public:
	void CheckQuestAchieve(const uint64_t quest_key, ServerCore::ContentsEntity* const key_entity)noexcept;

	bool AddQuest(const uint64_t quest_key, Quest* const quest)noexcept { 
		if (m_mapQuests.contains(quest_key))
		{
			return false;
		}
		m_mapQuests.emplace(quest_key, quest);
		return true;
	}
private:
	//TODO: 커스텀할당자 멀티맵 쓰기
	std::multimap<uint64_t, Quest*> m_mapQuests;
};

