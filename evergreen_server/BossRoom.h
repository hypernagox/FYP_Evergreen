#pragma once
#include "QuestRoom.h"

class BossRoom
	:public QuestRoom
{
public:

	virtual bool ProcessPartyQuest()noexcept override {
		return 1 == m_mon_count.fetch_sub(1);
	}
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	//virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept override;
	//virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept override;
	virtual void InitQuestField()noexcept override;

	S_ptr<ContentsEntity> CreateBoss()noexcept;
public:
	S_ptr<ContentsEntity> m_boss;
};

