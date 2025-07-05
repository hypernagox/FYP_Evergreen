#pragma once
#include "pch.h"
#include "NaviCell.h"
#include "Navigator.h"

namespace Common
{
	class NaviCell;
	class NavigationMesh;

	class NaviAgent
	{
	public:
		NaviCell GetPostCell(DirectX::SimpleMath::Vector3& pos)const noexcept;
		float GetSlope()const noexcept;
		DirectX::SimpleMath::Vector3 GetNaviPos(const DirectX::SimpleMath::Vector3& pos)noexcept;
		NavigationMesh* const GetNavMesh()const noexcept;
	public:
		Vector3 ForcedMovement(const Vector3& prev_pos, const Vector3& dest_pos) noexcept;
		Vector3& ForcedMovement(const Vector3 prev_pos, const Vector3& dest_pos, Vector3& out_pos) noexcept;

		void SetCurCell(DirectX::SimpleMath::Vector3& pos)noexcept;
		void SetCurCell(const NaviCell& cell)noexcept { m_curCell = cell; }
		auto& GetCurCell()noexcept { return m_curCell; }
		void SetNavMesh(const NAVI_MESH_TYPE nav_type)noexcept { m_navmesh_type = nav_type; }
		void SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos, Vector3& out_pos)noexcept;
	private:
		NaviCell m_curCell;
		NAVI_MESH_TYPE m_navmesh_type = {};
	};
}