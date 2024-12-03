#include "pch.h"
#include "QuestSystem.h"
#include "Quest.h"

QuestSystem::~QuestSystem() noexcept
{
	for (const auto [key, quest] : m_mapQuests)
	{
		ServerCore::xdelete<Quest>(quest);
	}
}

void QuestSystem::CheckQuestAchieve(const uint64_t quest_key, ServerCore::ContentsEntity* const key_entity) noexcept
{
	const auto [b, e] = m_mapQuests.equal_range(quest_key);
	const auto pOwner = GetOwnerEntityRaw();
	for (auto it = b; it != e;)
	{
		const auto& quest = it->second;

		if (quest->OnAchieve(key_entity,pOwner))
		{
			quest->OnReward(pOwner);
			it = m_mapQuests.erase(it);
		}
		else
		{
			++it;
		}
	}
}
