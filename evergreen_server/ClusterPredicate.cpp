#include "pch.h"
#include "ClusterPredicate.h"
#include "ClientSession.h"
#include "func.h"
#include "PositionComponent.h"
#include "TickTimer.h"
#include "HP.h"
#include "Interaction.h"

using namespace NagiocpX;

ClusterPredicate::ClusterPredicate()
{
}

ClusterPredicate::~ClusterPredicate()
{
}

bool ClusterPredicate::Filter4Session(const ContentsEntity* const a, const ContentsEntity* const b) const noexcept
{
	if (!a->IsValid() || !b->IsValid()) 
	{
		return false;
	}
	if (a->IsPendingClusterEntry() || b->IsPendingClusterEntry())
	{
		return false;
	}
	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;
	
	return  CommonMath::IsInDistanceDX(a_pos, b_pos, DISTANCE_FILTER);
}

bool ClusterPredicate::Filter4NPC(const ContentsEntity* const a, const ContentsEntity* const b) const noexcept
{
	if (!a->IsReadyAndValid() || !b->IsReadyAndValid())return false;
	
	auto a_pos = a->GetComp<PositionComponent>()->pos;
	auto b_pos = b->GetComp<PositionComponent>()->pos;

	a_pos.y = b_pos.y = 0.f;

	const auto dist = Vector3::DistanceSquared(a_pos, b_pos);
	const bool bRes = (DISTANCE_FILTER * DISTANCE_FILTER) >= dist;
	if (bRes)
	{
		if (const auto npc_timer = b->GetIocpComponent<NagiocpX::TickTimer>())
		{
			const uint32_t awake_dist = npc_timer->GetAwakeDistance();
			if (awake_dist >= dist)
				npc_timer->TryExecuteTimer(a->GetObjectID());
		}
	}

	return bRes;
}

S_ptr<SendBuffer> ClusterPredicate::CreateAddPacket(const ContentsEntity* const entity_ptr) const noexcept
{
	const auto& pEntity = entity_ptr->GetComp<PositionComponent>();
	// TODO: HP가 없는 오브젝트의 appear는 어떻게 할 까
	if (const auto hp = entity_ptr->GetComp<HP>())
		return Create_s2c_APPEAR_OBJECT(pEntity->GetOwnerObjectID(), (Nagox::Enum::GROUP_TYPE)entity_ptr->GetPrimaryGroupType<Nagox::Enum::GROUP_TYPE>(), entity_ptr->GetDetailType(), pEntity->GetPktPos()
			, hp->GetMaxHP(), hp->GetCurHP()
		);
	else if (const auto interaction = entity_ptr->GetComp<Interaction>())
	{
		return Create_s2c_APPEAR_OBJECT(pEntity->GetOwnerObjectID(), (Nagox::Enum::GROUP_TYPE)entity_ptr->GetPrimaryGroupType<Nagox::Enum::GROUP_TYPE>(), entity_ptr->GetDetailType(), pEntity->GetPktPos()
			, -1, interaction->GetInteractionType());
	}
	else
		return Create_s2c_APPEAR_OBJECT(pEntity->GetOwnerObjectID(), (Nagox::Enum::GROUP_TYPE)entity_ptr->GetPrimaryGroupType<Nagox::Enum::GROUP_TYPE>(), entity_ptr->GetDetailType(), pEntity->GetPktPos()
			, -1, -1);
}

S_ptr<SendBuffer> ClusterPredicate::CreateRemovePacket(const uint32_t obj_id) const noexcept
{
	return Create_s2c_REMOVE_OBJECT(obj_id);
}

S_ptr<SendBuffer> ClusterPredicate::CreateMovePacket(const ContentsEntity* const entity_ptr) const noexcept
{
	const auto pos_comp = entity_ptr->GetComp<PositionComponent>();
	return Create_s2c_MOVE(
		pos_comp->GetOwnerObjectID(),
		pos_comp->GetPktPos(),
		pos_comp->GetPktVel(),
		pos_comp->GetPktAccel(),
		pos_comp->body_angle,
		pos_comp->time_stamp);
}

