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
	void PostCheckQuestAchieve(NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity)noexcept;
	bool AddQuest(Quest* const quest)noexcept;
private:
	void CheckQuestAchieve(const NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity)noexcept;
private:
	// TODO: 바이트정렬
	Quest* m_arrQuests[NUM_OF_MAX_QUESTS] = { nullptr };
};

