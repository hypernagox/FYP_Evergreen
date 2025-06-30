#pragma once

class QuestRoom;

class PartyQuestTable
{
public:
	static void InitPartyQuestTable()noexcept;
	static inline S_ptr<QuestRoom> CreatePartyQuest(const int party_quest_id)noexcept { 
		if (party_quest_id >= g_party_quest_table.size())return {};
		return g_party_quest_table[party_quest_id]();
	}
private:
	using PartyQuestFactory = std::function<S_ptr<QuestRoom>(void)>;
	static inline  XVector<PartyQuestFactory> g_party_quest_table = {};
};

