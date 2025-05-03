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
		const float MAX_HEIGHT_DIFF = 30.25f * dt;
		constexpr const float MIN_HEIGHT_VAL = .12f;
		const auto cur_poly_ref = m_curCell.GetPolyRef();
		dtPolyRef p[10]{ cur_poly_ref };
		int v = 1;

		const auto prev_z = CommonMath::InverseZ(prev_pos);
		const auto post_z = CommonMath::InverseZ(post_pos);

		const auto nav_q = m_pNavMesh->GetNavMeshQuery();

		nav_q->moveAlongSurface(cur_poly_ref, &prev_z.x, &post_z.x, m_pNavMesh->GetNavFilter(), &out_pos.x, p, &v, 10);
		const auto post_poly_ref = p[v - 1];

		m_curCell.SetPolyRef(post_poly_ref);

		nav_q->getPolyHeight(post_poly_ref, &out_pos.x, &out_pos.y);
		
		const float height_diff = std::abs(out_pos.y - prev_z.y);

		if (prev_pos != Vector3::Zero && (MAX_HEIGHT_DIFF <= height_diff || MIN_HEIGHT_VAL >= out_pos.y))
		{
			m_curCell.SetPolyRef(cur_poly_ref);
			out_pos = prev_z;
			nav_q->getPolyHeight(cur_poly_ref, &out_pos.x, &out_pos.y);
		}

		CommonMath::InverseZ(out_pos);
	}
}