#include "pch.h"
#include "Death.h"
#include "NaviAgent_Common.h"
#include "Navigator.h"
#include "HP.h"
#include "CreateBuffer4Server.h"
#include "ClusterInfoHelper.h"
#include "IocpObject.h"
#include "TickTimer.h"

void MonsterDeath::ProcessDeath() noexcept
{
	const auto owner = GetOwnerEntityRaw();
	owner->TryOnDestroy();
}

void PlayerDeath::ProcessDeath() noexcept
{
	const auto owner = GetOwnerEntityRaw();
	const auto pkt = Create_s2c_PLAYER_DEATH(owner->GetObjectID(), { 0,0,0 });
	owner->GetComp<ServerCore::ClusterInfoHelper>()->BroadcastCluster(pkt);
}

