#include "pch.h"
#include "NaviAgent.h"
#include "Navigator.h"
#include "NaviCell.h"
#include "NavigationMesh.h"

namespace Common
{
	NaviCell NaviAgent::GetPostCell(DirectX::SimpleMath::Vector3& pos)const noexcept
	{
		return m_pNavMesh->GetNaviCell(pos);
	}

	DirectX::SimpleMath::Vector3 NaviAgent::GetNaviPos(const DirectX::SimpleMath::Vector3& pos) noexcept
	{
		const auto y = m_curCell.CalculateHeight(pos, m_pNavMesh);

		return { pos.x,y,pos.z };
	}

	void NaviAgent::SetCurCell(DirectX::SimpleMath::Vector3& pos) noexcept
	{
		m_curCell = GetPostCell(pos);
	}

	void NaviAgent::SetCellPos(const float dt, const Vector3& prev_pos, const Vector3& post_pos, Vector3& out_pos) noexcept
	{
		constexpr const float MIN_HEIGHT_VAL = .12f;
		const auto cur_poly_ref = m_curCell.GetPolyRef();
		dtPolyRef p[10]{ cur_poly_ref };
		int v = 1;

		const auto prev_z = CommonMath::InverseZ(prev_pos);
		const auto post_z = CommonMath::InverseZ(post_pos);

		const auto nav_q = m_pNavMesh->GetNavMeshQuery();

		dtPolyRef targetPoly = 0;

		Vector3 nearestPt = prev_z;
		
		constexpr const float extents[3] = { 2.f, 4.f, 2.f };

		dtStatus status = nav_q->findNearestPoly(&post_z.x, extents, m_pNavMesh->GetNavFilter(), &targetPoly, &nearestPt.x);

		if (!dtStatusSucceed(status) || 0 == targetPoly)
		{
			out_pos = prev_z;
			nav_q->getPolyHeight(cur_poly_ref, &out_pos.x, &out_pos.y);
			CommonMath::InverseZ(out_pos);
			return;
		}

		status = nav_q->moveAlongSurface(targetPoly, &prev_z.x, &post_z.x, m_pNavMesh->GetNavFilter(), &out_pos.x, p, &v, 10);

		if (!dtStatusSucceed(status))
		{
			out_pos = nearestPt;
			nav_q->getPolyHeight(targetPoly, &out_pos.x, &out_pos.y);
			m_curCell.SetPolyRef(targetPoly);
			CommonMath::InverseZ(out_pos);
			return;
		}

		const auto post_poly_ref = p[v - 1];

		m_curCell.SetPolyRef(post_poly_ref);

		nav_q->getPolyHeight(post_poly_ref, &out_pos.x, &out_pos.y);

		if (MIN_HEIGHT_VAL >= out_pos.y)
		{
			out_pos = nearestPt;
			nav_q->getPolyHeight(targetPoly, &out_pos.x, &out_pos.y);
			m_curCell.SetPolyRef(targetPoly);
		}

		CommonMath::InverseZ(out_pos);
	}

	Vector3 NaviAgent::ForcedMovement(const Vector3& prev_pos, const Vector3& dest_pos) noexcept
	{
		constexpr const float MIN_HEIGHT_VAL = 0.12f;

		const dtPolyRef cur_poly_ref = m_curCell.GetPolyRef();
		dtPolyRef visited_polys[10]{ cur_poly_ref };
		int visited_count = 1;

		const Vector3 prev_z = CommonMath::InverseZ(prev_pos);
		const Vector3 dest_z = CommonMath::InverseZ(dest_pos);

		const auto nav_q = m_pNavMesh->GetNavMeshQuery();
		const auto nav_filter = m_pNavMesh->GetNavFilter();

		float t = 1.0f;
		float hitNormal[3]{ 0,0,0 };

		const dtStatus ray_status = nav_q->raycast(
			cur_poly_ref, &prev_z.x, &dest_z.x,
			nav_filter, &t, hitNormal, visited_polys, &visited_count, 10
		);

		Vector3 adjusted_dest = dest_z;

		if (dtStatusSucceed(ray_status) && 1.0f > t)
		{
			adjusted_dest = prev_z + (dest_z - prev_z) * t;
		}

		Vector3 moved = prev_z;

		const dtStatus move_status = nav_q->moveAlongSurface(
			cur_poly_ref, &prev_z.x, &adjusted_dest.x,
			nav_filter, &moved.x, visited_polys, &visited_count, 10
		);

		dtPolyRef final_poly = cur_poly_ref;

		if (dtStatusFailed(move_status))
		{
			moved = prev_z + (adjusted_dest - prev_z);
			nav_q->getPolyHeight(cur_poly_ref, &moved.x, &moved.y);
		}
		else
		{
			final_poly = visited_polys[visited_count - 1];
			nav_q->getPolyHeight(final_poly, &moved.x, &moved.y);
		}

		if (moved.y <= MIN_HEIGHT_VAL)
		{
			moved = prev_z;
			nav_q->getPolyHeight(cur_poly_ref, &moved.x, &moved.y);
			final_poly = cur_poly_ref;
		}

		m_curCell.SetPolyRef(final_poly);
		CommonMath::InverseZ(moved);
		return moved;
	}

	Vector3& NaviAgent::ForcedMovement(const Vector3 prev_pos, const Vector3& dest_pos, Vector3& out_pos) noexcept
	{
		return out_pos = ForcedMovement(prev_pos, dest_pos);
	}
}