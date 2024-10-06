#include "pch.h"
#include "NaviAgent.h"
#include "Navigator.h"
#include "NaviMesh.h"
#include "NaviCell.h"

NaviAgent::NaviAgent()
{
	m_pNaviMesh = NAVIGATION->GetNavMesh(NUM_0);
	m_pNaviCell = m_pNaviMesh->FindCellContainingOrClosestPoint({});
}

const NaviCell* NaviAgent::GetPostCell(const DirectX::SimpleMath::Vector3& pos) const noexcept
{
	return m_pNaviMesh->FindRayIntersectingCellInNeighbourhoods(m_pNaviCell, pos, NAVI_DEPTH);
}

float NaviAgent::GetSlope() const noexcept
{
	return m_pNaviCell->CalculateSelfSlopeAngle();
}

float NaviAgent::GetSlope(const NaviCell* const cell) const noexcept
{
	return m_pNaviCell->CalculateSlopeAngleDifference(cell);
}

float NaviAgent::GetCellDist(const NaviCell* const cell) const noexcept
{
	return (m_pNaviCell->GetCellCenter() - cell->GetCellCenter()).LengthSquared();
}

DirectX::SimpleMath::Vector3 NaviAgent::GetNaviPos(const DirectX::SimpleMath::Vector3& pos) noexcept
{
	auto post_cell = GetPostCell(pos);
	if (nullptr == post_cell)
	{
		// 이번에 가야할 위치에 셀이 없음
		// 그나마 가까운셀로간다.
		post_cell = m_pNaviMesh->FindCellWithClosestCenter(pos);
	}
	m_pNaviCell = post_cell;
	return { pos.x,post_cell->CalculateY(pos.x,pos.z),pos.z };
}
