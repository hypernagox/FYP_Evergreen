#include "pch.h"
#include "ClusterPredicate.h"
#include "ClientSession.h"
#include "func.h"
#include "PositionComponent.h"
#include "TickTimer.h"

using namespace ServerCore;

ClusterPredicate::ClusterPredicate()
{
}

ClusterPredicate::~ClusterPredicate()
{
}

bool ClusterPredicate::Filter4Session(const ContentsEntity* const a, const ContentsEntity* const b) const noexcept
{
	if (!a->IsValid() || !b->IsValid())return false;
	// if (a->IsPendingClusterEntry() || b->IsPendingClusterEntry())return false;// 이거는 상황보고 다시 해야할수도있음 일단 패킷핸들러에서만 체크해봄

	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;

	const int dx = (int)(a_pos.x - b_pos.x);
	const int dy = (int)(a_pos.y - b_pos.y);
	const int dz = (int)(a_pos.z - b_pos.z);
	//return true;
	return ((50 * 50) >= (dx * dx + dy * dy + dz * dz));
}

bool ClusterPredicate::Filter4NPC(const ContentsEntity* const a, const ContentsEntity* const b) const noexcept
{
	if (!a->IsValid() || !b->IsValid())return false;
	if (a->IsPendingClusterEntry() || b->IsPendingClusterEntry())return false;

	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;

	const int dx = (int)(a_pos.x - b_pos.x);
	const int dy = (int)(a_pos.y - b_pos.y);
	const int dz = (int)(a_pos.z - b_pos.z);

	const uint32_t dist = ((dx * dx + dy * dy + dz * dz));
	const bool bRes = (50 * 50) >= dist;

	if (bRes)
	{
		if (const auto npc_timer = b->GetIocpComponent<ServerCore::TickTimer>())
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

	return Create_s2c_APPEAR_OBJECT(pEntity->GetOwnerObjectID(), (Nagox::Enum::GROUP_TYPE)entity_ptr->GetPrimaryGroupType<Nagox::Enum::GROUP_TYPE>(), entity_ptr->GetDetailType(), ToFlatVec3(pEntity->pos));
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
		ToFlatVec3(pos_comp->pos),
		ToFlatVec3(pos_comp->vel),
		ToFlatVec3(pos_comp->accel),
		pos_comp->body_angle,
		pos_comp->time_stamp);
}

