#include "pch.h"
#include "Death.h"
#include "NaviAgent_Common.h"
#include "Navigator.h"
#include "HP.h"
#include "CreateBuffer4Server.h"
#include "ClusterInfoHelper.h"
#include "IocpObject.h"
#include "TickTimer.h"
#include "Cluster.h"
#include "FieldMgr.h"
#include "Field.h"
#include "EntityFactory.h"
#include "PositionComponent.h"
#include "ClusterPredicate.h"
#include "DropTable.h"
#include "QuestRoom.h"

void MonsterDeath::ProcessDeath() noexcept
{
	// TODO: 매직넘버 0
	// TODO: 확률
	const auto owner = GetOwnerEntityRaw();
	if (const auto item_table = owner->GetComp<DropTable>())
	{
		item_table->TryCreateItem();
	}
	if (owner->GetClusterFieldInfo().clusterInfo.fieldID == -1)
	{
		static_cast<QuestRoom*>(owner->GetClusterFieldInfo().curFieldPtr)->CheckPartyQuestState();
	}
	owner->TryOnDestroy();
}

void PlayerDeath::ProcessDeath() noexcept
{
	const auto owner = GetOwnerEntityRaw();
	const auto pkt = Create_s2c_PLAYER_DEATH(owner->GetObjectID(), { 0,0,0 });
	owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(pkt);
}

void HarvestDeath::ProcessDeath() noexcept
{
	const auto owner = GetOwnerEntityRaw();
	owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(Create_s2c_GET_HARVEST(owner->GetObjectID()));
	owner->TryOnDestroy();
}
