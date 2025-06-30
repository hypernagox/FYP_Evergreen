#include "pch.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "PositionComponent.h"

void NaviAgent::Init(const Vector3& pos, Common::NavigationMesh* const pNavMesh) noexcept
{
	auto& cur_pos = m_posComp->pos;
	cur_pos = pos;
	m_agent.SetNavMesh(pNavMesh);
	SetPos(pos);
}

void NaviAgent::InitRandPos(Common::NavigationMesh* const pNavMesh) noexcept
{
	m_agent.SetNavMesh(pNavMesh);
	pNavMesh->GetRandomPos(m_posComp->pos, m_agent.GetCurCell());

	//dtCrowdAgentParams params = InitParam();

	auto pos = m_posComp->pos;
	CommonMath::InverseZ(pos);
	//m_my_idx = pNavMesh->GetCrowd()->addAgent(&pos.x, &params);
}

void NaviAgent::SetPos(const Vector3& pos) noexcept
{
	auto& cur_pos = m_posComp->pos;
	cur_pos = pos;
	m_agent.GetCurCell() = m_agent.GetNavMesh()->GetNaviCell(cur_pos);
}

Common::NavigationMesh* const NaviAgent::GetNavMesh() noexcept
{
	return m_agent.GetNavMesh();
}

void NaviAgent::SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos) noexcept
{
	m_agent.SetCellPos(dt, prev_pos, post_pos, m_posComp->pos);
}

float NaviAgent::ApplyPostPosition(const Vector3& dir, const float speed, const float dt) noexcept
{
	const auto dir_ = CommonMath::Normalized(dir) * speed * dt;
	const auto& prev_pos = m_posComp->pos;
	const auto post_pos = prev_pos + dir_;
	SetCellPos(dt, prev_pos, post_pos);
	return dir_.Length();
}

void NaviAgent::ForcedMovement(const Vector3 dir, const float movement_dist) noexcept
{
	auto& cur_pos = m_posComp->pos;
	const auto dest_pos = cur_pos + dir * movement_dist;
	m_agent.ForcedMovement(cur_pos, dest_pos, cur_pos);
}

void NaviAgent::ProcessCleanUp() noexcept
{
}
