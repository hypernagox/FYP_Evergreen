#include "pch.h"
#include "Projectile.h"
#include "TaskTimerMgr.h"
#include  "Collider_Common.h"
#include "PositionComponent.h"
#include "HP.h"
#include "MoveBroadcaster.h"
#include "Cluster.h"

void Projectile::SelectObjList(const XVector<const ContentsEntity*>& vec_) noexcept
{
	NagiocpX::CreateEntityCompArr<Collider>(m_obj_list, vec_);
}

NagiocpX::ROUTINE_RESULT Projectile::Routine() noexcept
{
	// TODO: 개 쌉 하드코딩 + 매직넘버
	DO_BENCH_GLOBAL_THIS_FUNC;
	m_timer.Update();
	const auto dt = m_timer.GetDT();
	const auto pos = m_pos;
	constexpr const float RADIUS = 1.5f;
	bool isHit = false;
	const DirectX::BoundingSphere s{ pos,RADIUS };
	const auto& owner = m_owner;

	for (const auto& [obj,col] : m_obj_list)
	{
		if (obj->GetPrimaryGroupType() == Nagox::Enum::GROUP_TYPE_HARVEST)continue;
		if (col->GetCollider()->IsIntersect(s))
		{
			obj->GetComp<HP>()->PostDoDmg(1, owner);
			isHit = true;
		}
	}

	const auto delta = m_speed * dt;
	m_accDist += delta.LengthSquared();
	if (isHit)return NagiocpX::ROUTINE_RESULT::STOP;
	if (MAX_DIST <= m_accDist)return NagiocpX::ROUTINE_RESULT::STOP;
	m_pos = m_pos + m_speed * dt;
	return NagiocpX::ROUTINE_RESULT::STILL_RUNNIG;
}

void PlayerProjectile::StartRoutine() noexcept
{
	// TODO 투사체 종류
	auto pkt = Create_s2c_FIRE_PROJ(
		m_owner->GetObjectID(),
		m_proj_id,
		0,
		::ToFlatVec(m_pos), ::ToFlatVec(m_speed)
	);

	m_owner->GetComp<NagiocpX::MoveBroadcaster>()->BroadcastPacket(pkt);
	m_owner->GetSession()->SendAsync(std::move(pkt));
}

void PlayerProjectile::ProcessRemove() noexcept
{
	auto pkt = Create_s2c_REMOVE_OBJECT(m_proj_id);
	m_owner->GetComp<NagiocpX::MoveBroadcaster>()->BroadcastPacket(pkt);
	m_owner->GetSession()->SendAsync(std::move(pkt));
}

void MonProjectile::StartRoutine() noexcept
{
	// TODO 투사체 종류
	auto pkt = Create_s2c_FIRE_PROJ(
		m_owner->GetObjectID(),
		m_proj_id,
		0,
		::ToFlatVec(m_pos), ::ToFlatVec(m_speed)
	);
	for (const auto& [obj, col] : m_obj_list) {
		obj->GetSession()->SendAsync(pkt);
	}
}

void MonProjectile::ProcessRemove() noexcept
{
	auto pkt = Create_s2c_REMOVE_OBJECT(m_proj_id);
	for (const auto& [obj, col] : m_obj_list) {
		obj->GetSession()->SendAsync(pkt);
	}
}
