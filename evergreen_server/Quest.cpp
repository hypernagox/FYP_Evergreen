#include "pch.h"
#include "Quest.h"
#include "Session.h"

bool KillFoxQuest::OnAchieve(ServerCore::ContentsEntity* const key_entity, ServerCore::ContentsEntity* const clear_entity) noexcept
{
    if (const auto session = clear_entity->GetSession())
    {
        session->SendAsync(Create_s2c_CLEAR_QUEST(0, false));
    }
    return m_clearCount <= ++m_curCount;
}

void KillFoxQuest::OnReward(ServerCore::ContentsEntity* const clear_entity) noexcept
{
    if (const auto session = clear_entity->GetSession())
    {
        session->SendAsync(Create_s2c_CLEAR_QUEST(0, true));
    }
}
