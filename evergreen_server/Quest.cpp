#include "pch.h"
#include "Quest.h"
#include "KillMonsterQuest.h"
#include "Inventory.h"
#include "CommonQuestTable.h"
#include "DataRegistry.h"

Quest* const Quest::CreateQuest(const uint64_t quest_id) noexcept
{
    static const std::function<Quest* (void)> g_quest_list[] =
    {
        NagiocpX::xnew<KillFoxQuest>,
        NagiocpX::xnew<KillBearQuest>,
        NagiocpX::xnew<KillBearFoxQuest>,
        NagiocpX::xnew<ManyFoxKillQuest>,
    };

    return g_quest_list[quest_id]();
}
void Quest::ProcessReward(NagiocpX::ContentsEntity* const clear_entity, const uint64_t quest_id) noexcept
{
    const auto& quest_reward_info = Common::CommonQuestTable::GetCommonQuestInfo(quest_id);
    const auto entity_inventory = clear_entity->GetComp<Inventory>();
    for (const auto& [item_name, item_id, amount] : quest_reward_info.reward_info)
    {
        entity_inventory->AddItem(item_id, amount);
    }
}
