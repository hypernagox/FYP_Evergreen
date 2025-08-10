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
	Quaternion rotation = Quaternion::CreateFromYawPitchRoll(move_data.body_angleY * DEG2RAD + PI, 0.0f, 0.0f);

	// TODO: 보스는 서버에서 위치말곤 관리를 안할거라 보스의 경우 여기서 하드코딩이 좀 필요할 가능성 있음
	owner_player->SetPosition(move_data.pos);
	owner_player->SetVelocity(move_data.vel);
	owner_player->SetAcceleration(move_data.accel);
	owner_player->SetRotation(rotation);
	owner_player->SetForward(Vector3::Transform(Vector3::Forward, rotation));

	
	// TODO: 본인 말고 다른 오브젝트를 지형보정 해주던 부분인데 고민을 해보도록해요
	//const auto pos = root_obj->GetComponent<ServerObject>()->m_pNaviAgent->GetNaviPos(root_obj->GetTransform()->GetLocalPosition());
	//root_obj->GetTransform()->SetLocalPosition(pos);
}

void MoveInterpolator::UpdateNewMoveData(const Nagox::Protocol::s2c_MOVE& pkt_) noexcept
{
	const auto owner_player = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	
	const auto pos = ToOriginVec3(pkt_.pos());
	const auto vel = ToOriginVec3(pkt_.vel());
	const auto accel = ToOriginVec3(pkt_.accel());
	const auto angle = pkt_.body_angle();
	const Quaternion rotation = Quaternion::CreateFromYawPitchRoll(angle * DEG2RAD + PI, 0.0f, 0.0f);
	
	constexpr const float MIN_DT = 0.1f;

	const auto dt = std::min(
		(NetMgr(ServerTimeMgr)->GetDtForDeadReckoningSeconds(pkt_.time_stamp())),
		MIN_DT
	);
	
	//std::cout << dt << std::endl;
	
	const Vector3 vFutureVel = vel + accel * dt;
	const Vector3 vFuturePos = pos + vel * dt + 0.5f * accel * dt * dt;

	auto move_data = m_interpolator.GetInterPolatedData();
	m_interpolator.UpdateNewData(MoveData{ vFuturePos, angle, vFutureVel, accel });
	m_interpolator.GetCurData().pos = move_data.pos;
	m_interpolator.GetCurData().vel = move_data.vel;
	// m_interpolator.GetCurData().body_angleY = move_data.body_angleY;

	// TODO: 여기서 셋포지션하면 이상할 것 같은데 일단 관찰
	owner_player->SetPosition(pos);
	owner_player->SetVelocity(vel);
	owner_player->SetAcceleration(accel);
	owner_player->SetRotation(rotation);
	owner_player->SetForward(Vector3::Transform(Vector3::Forward, rotation));

	//m_interpolator.GetCurData().pos = owner_player->GetTransform()->GetWorldPosition();
	//if (owner_player->GetVelocity().LengthSquared() == 0.f)
	//{
	//	m_interpolator.GetCurData().vel = vel*2;
	//	m_interpolator.GetCurData().accel = accel*2;
	//	
	//}
	//m_interpolator.GetCurData().accel = owner_player->GetAcceleration();

	// TOOD: 지형보정 방식은 앞으로 바뀔 듯 해요
	// auto move_data = m_interpolator.GetInterPolatedData();
	//root_obj->GetTransform()->SetLocalPosition(move_data.pos);
}

void MoveInterpolator::UpdateNewMoveData(const Vector3& position, float viewAngleY) noexcept
{
	auto move_data = m_interpolator.GetInterPolatedData();
	m_interpolator.UpdateNewData(MoveData{ position, viewAngleY, {}, {} });
	m_interpolator.GetCurData().pos = move_data.pos;
	m_interpolator.GetCurData().body_angleY = move_data.body_angleY;
}

void MoveInterpolator::UpdateNewMoveDataOnlyPos(const Vector3& dest_pos) noexcept
{
	const auto owner_player = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	auto move_data = m_interpolator.GetInterPolatedData();

	if (Vector3::DistanceSquared(owner_player->GetPosition(), dest_pos) < 1e-3f)
	{
		return; // 위치가 같으면 업데이트 안함
	}
	Vector3 delta = dest_pos - owner_player->GetPosition();
	delta.y = 0.0f;
	delta.Normalize();
	float bodyAngleY = std::atan2(delta.x, delta.z) * RAD2DEG;

	m_interpolator.UpdateNewData(MoveData{ dest_pos, bodyAngleY, {}, {} });
	m_interpolator.GetCurData().pos = move_data.pos;
	m_interpolator.GetCurData().body_angleY = move_data.body_angleY;
}

void MoveInterpolator::UpdateForcedMoveData(const Vector3& pos) noexcept
{
	const auto owner_player = m_owner->GetSceneObject()->GetComponent<EntityMovement>();
	m_interpolator.GetCurData().pos = pos;
	m_interpolator.GetNewData().pos = pos;
	owner_player->SetPosition(pos);
}
