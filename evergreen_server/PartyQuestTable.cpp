#include "pch.h"
#include "PartyQuestTable.h"
#include "QuestRoom.h"

void PartyQuestTable::InitPartyQuestTable() noexcept
{
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<TutorialGuardQuest>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<NPCGuardQuest2>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<FoxQuest>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<GoblinQuest>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<BearQuest>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<InvadeQuest_1>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<InvadeQuest_2>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<CombinationBattleQuest>);
	}
	{
		g_party_quest_table.emplace_back(NagiocpX::MakeShared<NexusQuest>);
	}
}
