#include "pch.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "MoveInterpolator.h"
#include "NaviAgent.h"

void MoveInterpolator::Update() noexcept
{
	const auto& root_obj = m_owner->GetSceneObject();
	const auto owner_player = root_obj->GetComponent<EntityMovement>();

	const auto move_data = m_interpolator.GetInterPolatedData();
	
	owner_player->SetVelocity(move_data.vel);
	owner_player->SetAcceleration(move_data.accel);
	root_obj->GetTransform()->SetLocalPosition(move_data.pos);
	
	// TODO: 본인 말고 다른 오브젝트를 지형보정 해주던 부분인데 고민을 해보도록해요
	//const auto pos = root_obj->GetComponent<ServerObject>()->m_pNaviAgent->GetNaviPos(root_obj->GetTransform()->GetLocalPosition());
	//root_obj->GetTransform()->SetLocalPosition(pos);
	owner_player->SetRotation(Quaternion::CreateFromYawPitchRoll(move_data.body_angleY * DEG2RAD + PI, 0.0f, 0.0f));
}

void MoveInterpolator::UpdateNewMoveData(const Nagox::Protocol::s2c_MOVE& pkt_) noexcept
{
	const auto owner_player = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	
	const auto pos = ToOriginVec3(pkt_.pos());
	const auto vel = ToOriginVec3(pkt_.vel());
	const auto accel = ToOriginVec3(pkt_.accel());
	
	//const auto dt = (NetHelper::GetSystemTimeStampMilliseconds() - pkt_.time_stamp()) / 2000.f;

	const auto dt = DT;

	const Vector3 vFutureVel = vel + accel * dt;

	const Vector3 vFuturePos = pos + vFutureVel * dt + accel * dt * dt * 0.5f;
	auto move_data = m_interpolator.GetInterPolatedData();

	m_interpolator.UpdateNewData(MoveData{ vFuturePos ,pkt_.body_angle(),vFutureVel,accel });
	m_interpolator.GetCurData().pos = move_data.pos;
	const auto& root_obj = owner_player->GetSceneObject();
	// TOOD: 지형보정 방식은 앞으로 바뀔 듯 해요
	// auto move_data = m_interpolator.GetInterPolatedData();
	//root_obj->GetTransform()->SetLocalPosition(move_data.pos);
}
