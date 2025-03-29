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
#include "DropItem.h"
#include "ClusterPredicate.h"

void MonsterDeath::ProcessDeath() noexcept
{
	// TODO: 매직넘버 0
	const auto owner = GetOwnerEntityRaw();
	EntityBuilder b;
	const auto pos = owner->GetComp<PositionComponent>()->pos;
	b.group_type = Nagox::Enum::GROUP_TYPE_DROP_ITEM;
	b.obj_type = 0;
	b.x = pos.x;
	b.y = pos.y;
	b.z = pos.z;
	auto item = EntityFactory::CreateDropItem(b);
	if (const auto item_ptr = item->GetComp<DropItem>())
	{
		if (item_ptr->TryDropItem())
		{
			const auto temp_ptr = item.get();
			ClusterPredicate p;
			Mgr(FieldMgr)->GetField(0)->EnterFieldNPC(
				std::move(item)
			);
			owner->GetCurCluster()->Broadcast(p.CreateAddPacket(temp_ptr));
		}
	}
	owner->TryOnDestroy();
}

void PlayerDeath::ProcessDeath() noexcept
{
	const auto owner = GetOwnerEntityRaw();
	const auto pkt = Create_s2c_PLAYER_DEATH(owner->GetObjectID(), { 0,0,0 });
	owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(pkt);
}

