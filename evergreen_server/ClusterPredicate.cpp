#include "pch.h"
#include "ClusterPredicate.h"
#include "ClientSession.h"
#include "func.h"
#include "PositionComponent.h"
#include "TickTimer.h"
#include "HP.h"

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
		//std::cout << "Invalid at\n";
		//std::cout << "A: " << a->GetObjectID() << std::endl;
		//std::cout << "B: " << b->GetObjectID() << std::endl;
		return false;
	}
	if (a->IsPendingClusterEntry() || b->IsPendingClusterEntry())
	{
		//std::cout << "Pending at\n";
		//std::cout << "A: " << a->GetObjectID() << std::endl;
		//std::cout << "B: " << b->GetObjectID() << std::endl;
		return false;
	}
	// 이거는 상황보고 다시 해야할수도있음 일단 패킷핸들러에서만 체크해봄

	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;

	//const int dx = (int)(a_pos.x - b_pos.x);
	//const int dy = (int)(a_pos.y - b_pos.y);
	//const int dz = (int)(a_pos.z - b_pos.z);
	//return true;4
	const auto res = CommonMath::IsInDistanceDX(a_pos, b_pos, 50);
	if (!res)
	{
		std::cout << "TTTTTTTTTTTTTTTTTTTTTTTTt\n";
		std::cout << CommonMath::GetDistPowDX(a_pos, b_pos) << std::endl;
	}
	return res;
	//return ((50 * 50) >= (dx * dx + dy * dy + dz * dz));
}

bool ClusterPredicate::Filter4NPC(const ContentsEntity* const a, const ContentsEntity* const b) const noexcept
{
	if (!a->IsReadyAndValid() || !b->IsReadyAndValid())return false;
	
	const auto a_pos = a->GetComp<PositionComponent>()->pos;
	const auto b_pos = b->GetComp<PositionComponent>()->pos;

	//const int dx = (int)(a_pos.x - b_pos.x);
	//const int dy = (int)(a_pos.y - b_pos.y);
	//const int dz = (int)(a_pos.z - b_pos.z);
	//
	//const uint32_t dist = ((dx * dx + dy * dy + dz * dz));
	const auto dist = Vector3::DistanceSquared(a_pos, b_pos);
	const bool bRes = (50 * 50) >= dist;
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

