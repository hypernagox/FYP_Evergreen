#pragma once
#include "ContentsComponent.h"

class Quest;

class QuestSystem
	:public ContentsComponent
{
public:
	static constexpr const uint8_t MAX_NUM_OF_QUESTS = 10;
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(QuestSystem)
public:
	virtual ~QuestSystem()noexcept;
public:
	void PostCheckQuestAchieve(NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity)noexcept;
	bool AddQuest(Quest* const quest)noexcept;
private:
	void CheckQuestAchieve(const NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity)noexcept;
private:
	// TODO: 바이트정렬
	Quest* m_arrQuests[MAX_NUM_OF_QUESTS] = { nullptr };
};

