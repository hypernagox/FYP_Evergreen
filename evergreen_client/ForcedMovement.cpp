#include "pch.h"
#include "ForcedMovement.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "MoveInterpolator.h"

void ForcedMovement::Update()noexcept
{
	//return;
	//if (!m_forcedMovementFlag)return;
	//const auto navi = m_owner->GetComponent<ServerObject>()->m_pNaviAgent;
	//
	//const auto movement_dist = m_speed * DT;
	//const auto movement = m_dir * movement_dist;
	//
	//const auto transform = m_owner->GetTransform();
	//const auto cur_pos = transform->GetWorldPosition();
	//const auto new_pos = cur_pos + movement;
	//Vector3 moved = cur_pos;
	//navi->SetCellPos(DT, cur_pos, new_pos, moved);
	//transform->SetLocalPosition(moved);
	//m_dist -= (moved-cur_pos).Length();
	//if (0.f >= m_dist) 
	//{ 
	//	m_forcedMovementFlag = false; 
	//	m_owner->GetComponent<EntityMovement>()->m_factor = 1.f;
	//	m_owner->GetComponent<EntityMovement>()->SetFriction(40.f);
	//}
}

//void ForcedMovement::SetForcedMovement(const Nagox::Protocol::s2c_FORCED_MOVEMENT& forced_movement) noexcept
//{
//	//return;
//	//m_forcedMovementFlag = true;
//	//const auto start_pos = ToOriginVec3(forced_movement.start_pos());
//	//const auto dest_pos = ToOriginVec3(forced_movement.dest_pos());
//	//m_speed = forced_movement.speed();
//	//const auto dir = dest_pos - start_pos;
//	//m_dir = CommonMath::Normalized(dir);
//	//m_owner->GetComponent<EntityMovement>()->SetVelocity(m_dir * m_speed);
//	//m_dist = dir.Length();
//	//auto& interpolator = m_owner->GetComp<MoveInterpolator>()->GetInterpolatorConcrete();
//	//interpolator.GetNewData().vel = m_dir * m_speed;
//	//interpolator.GetNewData().pos = dest_pos;
//	//interpolator.GetCurData().vel = m_dir * m_speed;
//	//auto data = interpolator.GetNewData();
//	//data.pos = dest_pos;
//	//data.vel = m_dir * m_speed;
//	//interpolator.UpdateNewData(data);
//	//m_owner->GetComponent<EntityMovement>()->SetFriction(0.f);
//	//m_owner->GetComponent<EntityMovement>()->m_factor = 10.f;
//}
