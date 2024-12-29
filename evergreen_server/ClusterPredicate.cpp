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

bool ClusterPredicate::ClusterHuristicFunc2Session(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b) noexcept
{
	if (!a->IsValid() || !b->IsValid())return false;
	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;

	const int dx = (int)(a_pos.x - b_pos.x);
	const int dy = (int)(a_pos.y - b_pos.y);
	const int dz = (int)(a_pos.z - b_pos.z);
	//return true;
	return ((50 * 50) >= (dx * dx + dy * dy + dz * dz));
}

bool ClusterPredicate::ClusterHuristicFunc2NPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b) noexcept
{
	if (!a->IsValid() || !b->IsValid())return false;
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
				npc_timer->TryExecuteTimer(a);
		}
	}

	return bRes;
}

S_ptr<SendBuffer> ClusterPredicate::ClusterAddPacketFunc(const ServerCore::ContentsEntity* const p) noexcept
{
	const auto& pEntity = p->GetComp<PositionComponent>();

	return Create_s2c_APPEAR_OBJECT(pEntity->GetOwnerObjectID(), (Nagox::Enum::GROUP_TYPE)p->GetObjectType(), p->GetObjectTypeInfo(), ToFlatVec3(pEntity->pos));
}

S_ptr<SendBuffer> ClusterPredicate::ClusterRemovePacketFunc(const uint32_t obj_id) noexcept
{
	return Create_s2c_REMOVE_OBJECT(obj_id);
}

S_ptr<SendBuffer> ClusterPredicate::ClusterMovePacketFunc(const ServerCore::ContentsEntity* const p) noexcept
{
	const auto pos_comp = p->GetComp<PositionComponent>();
	return Create_s2c_MOVE(
		pos_comp->GetOwnerObjectID(),
		ToFlatVec3(pos_comp->pos),
		ToFlatVec3(pos_comp->vel),
		ToFlatVec3(pos_comp->accel),
		pos_comp->body_angle,
		pos_comp->time_stamp);
}

void ClusterPredicate::TryNotifyNPC(const ServerCore::ContentsEntity* const a, const ServerCore::ContentsEntity* const b) noexcept
{
	b->GetIocpComponent<ServerCore::TickTimer>()->TryExecuteTimer(a);
}
