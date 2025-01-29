#include "pch.h"
#include "Projectile.h"
#include "TaskTimerMgr.h"
#include  "Collider_Common.h"
#include "PositionComponent.h"
#include "HP.h"
#include "MoveBroadcaster.h"
#include "Cluster.h"

void Projectile::StartRoutine() noexcept
{
	auto pkt = Create_s2c_FIRE_PROJ(m_proj_id, ::ToFlatVec3(m_pos), ::ToFlatVec3(m_speed));
	m_owner->GetSession()->SendAsync(pkt);
	m_owner->GetComp<ServerCore::MoveBroadcaster>()->BroadcastPacket(std::move(pkt));
	m_timer.Update();
}

ServerCore::ROUTINE_RESULT Projectile::Routine() noexcept
{
	// TODO: 개 쌉 하드코딩 + 매직넘버
	m_timer.Update();
	const auto dt = m_timer.GetDT();
	const auto pos = m_pos;
	constexpr const float RADIUS = 1.5f;
	bool isHit = false;
	for (const auto& [mon,pos_comp] : m_mon_list)
	{
		if (!mon->IsValid() || mon->IsPendingClusterEntry())continue;
		if (const auto pCol = mon->GetComp<Collider>())
		{
			const auto owner = pCol->GetOwnerEntity();
			const auto mon_pos = pos_comp->pos;
			constexpr auto rr = RADIUS + RADIUS;
			if(ServerCore::IsInDistance(&pos.x, &mon_pos.x, rr))
			{
				owner->GetComp<HP>()->PostDoDmg(1, m_owner);
				isHit = true;
			}
		}
	}
	const auto delta= m_speed * dt;
	m_accDist += delta.LengthSquared();
	if (isHit)return ServerCore::ROUTINE_RESULT::STOP;
	if (MAX_DIST <= m_accDist)return ServerCore::ROUTINE_RESULT::STOP;
	m_pos = m_pos + m_speed * dt;
	return ServerCore::ROUTINE_RESULT::STILL_RUNNIG;
}

void Projectile::ProcessRemove() noexcept
{
	auto pkt = Create_s2c_REMOVE_OBJECT(m_proj_id);
	m_owner->GetSession()->SendAsync(pkt);
	m_owner->GetComp<ServerCore::MoveBroadcaster>()->BroadcastPacket(std::move(pkt));
}
