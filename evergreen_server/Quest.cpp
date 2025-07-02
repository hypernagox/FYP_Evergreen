#include "pch.h"
#include "Quest.h"
#include "KillMonsterQuest.h"

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
