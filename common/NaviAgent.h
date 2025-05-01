#pragma once
#include "pch.h"
#include "NaviCell.h"

namespace Common
{
	class NaviCell;
	class NavigationMesh;

	class NaviAgent
	{
	public:
		NaviCell GetPostCell(DirectX::SimpleMath::Vector3& pos)const noexcept;
		float GetSlope()const noexcept { return m_curCell.GetSlopeAngle(m_pNavMesh); }
		DirectX::SimpleMath::Vector3 GetNaviPos(const DirectX::SimpleMath::Vector3& pos)noexcept;
		const auto GetNavMesh()const noexcept { return m_pNavMesh; }
	public:
		void SetCurCell(DirectX::SimpleMath::Vector3& pos)noexcept;
		void SetCurCell(const NaviCell& cell)noexcept { m_curCell = cell; }
		auto& GetCurCell()noexcept { return m_curCell; }
		void SetNavMesh(NavigationMesh* const pNavMesh)noexcept { m_pNavMesh = pNavMesh; }
		void SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos, Vector3& out_pos)noexcept;
	private:
		NaviCell m_curCell;
		NavigationMesh* m_pNavMesh;
	};
}