#include "pch.h"
#include "QuestSystem.h"
#include "Quest.h"
#include "Queueabler.h"

QuestSystem::~QuestSystem() noexcept
{
	for (uint8_t i = 0; i < MAX_NUM_OF_QUESTS; ++i)
	{
		const auto q = m_arrQuests[i];
		if (q)NagiocpX::xdelete<Quest>(q);
	}
}

void QuestSystem::PostCheckQuestAchieve(NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity) noexcept
{
	GetOwnerEntityRaw()->GetQueueabler()->EnqueueAsyncPushOnly(&QuestSystem::CheckQuestAchieve, this, std::move(key_entity));
}

bool QuestSystem::AddQuest(Quest* const quest) noexcept
{
	uint8_t temp[MAX_NUM_OF_QUESTS] = { 0 };
	uint8_t num_of_temp = 0;
	const auto quest_key = quest->GetQuestKey();
	for (uint8_t i = 0; i < MAX_NUM_OF_QUESTS; ++i)
	{
		const auto q = m_arrQuests[i];
		if (q)
		{
			if (q->GetQuestKey() == quest_key)
				return false;
		}
		else
			temp[num_of_temp++] = i + 1;
	}
	for (uint8_t i = 0; i < MAX_NUM_OF_QUESTS; ++i)
	{
		if (0 == temp[i])break;
		m_arrQuests[temp[i] - 1] = quest;
		return true;
	}
	return false;
}

void QuestSystem::CheckQuestAchieve(const NagiocpX::S_ptr<NagiocpX::ContentsEntity> key_entity) noexcept
{
	const auto pOwner = GetOwnerEntityRaw();
	const auto key_entity_raw = key_entity.get();
	for (uint8_t i = 0; i < MAX_NUM_OF_QUESTS; ++i)
	{
		const auto q = m_arrQuests[i];
		if (q && q->OnAchieve(key_entity_raw, pOwner))
		{
			m_arrQuests[i] = nullptr;
			q->OnReward(pOwner);
			NagiocpX::xdelete<Quest>(q);
		}
	}
}
