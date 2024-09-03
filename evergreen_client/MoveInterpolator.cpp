#include "pch.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "MoveInterpolator.h"

extern void SetTerrainPos(const std::shared_ptr<SceneObject>& p);

void MoveInterpolator::Update() noexcept
{
	const auto& root_obj = m_owner->GetSceneObject();
	const auto owner_player = root_obj->GetComponent<EntityMovement>();

	const auto move_data = m_interpolator.GetInterPolatedData();
	
	root_obj->GetTransform()->SetLocalPosition(move_data.pos);
	SetTerrainPos(root_obj);
	owner_player->SetRotation(Quaternion::CreateFromYawPitchRoll(move_data.body_angleY * DEG2RAD + PI, 0.0f, 0.0f));
}

void MoveInterpolator::UpdateNewMoveData(const Nagox::Protocol::s2c_MOVE& pkt_) noexcept
{
	const auto owner_player = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	
	const auto pos = ToOriginVec3(pkt_.pos());
	const auto vel = ToOriginVec3(pkt_.vel());
	const auto accel = ToOriginVec3(pkt_.accel());

	const Vector3 vFutureVel = vel + accel * DT;

	const Vector3 vFuturePos = pos + vFutureVel * DT + accel * DT * DT * 0.5f;
	
	m_interpolator.UpdateNewData(MoveData{ vFuturePos ,pkt_.body_angle() });
	const auto& root_obj = owner_player->GetSceneObject();
	const auto move_data = m_interpolator.GetInterPolatedData();
	root_obj->GetTransform()->SetLocalPosition(move_data.pos);
	SetTerrainPos(root_obj);
}
