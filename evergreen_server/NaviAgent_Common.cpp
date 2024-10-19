#include "pch.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "PositionComponent.h"

void NaviAgent::Init(const Vector3& pos, Common::NavigationMesh* const pNavMesh) noexcept
{
	auto& cur_pos = m_posComp->pos;
	cur_pos = pos;
	m_agent.SetNavMesh(pNavMesh);
	m_agent.GetCurCell() = pNavMesh->GetNaviCell(cur_pos);
}

void NaviAgent::InitRandPos(Common::NavigationMesh* const pNavMesh) noexcept
{
	m_agent.SetNavMesh(pNavMesh);
	pNavMesh->GetRandomPos(m_posComp->pos, m_agent.GetCurCell());
}

void NaviAgent::SetCellPos(const Vector3& prev_pos, const Vector3& post_pos) noexcept
{
	m_agent.SetCellPos(prev_pos, post_pos, m_posComp->pos);
}
