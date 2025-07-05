#include "pch.h"
#include "BossRoom.h"

void BossRoom::InitQuestField() noexcept
{
	Broadcast2PartyMembers(Create_s2c_BOSS_ROOM_ENTER());
}
