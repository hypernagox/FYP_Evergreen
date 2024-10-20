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

	void NaviAgent::SetCellPos(const Vector3& prev_pos, const Vector3& post_pos, Vector3& out_pos) noexcept
	{
		// TODO: �����ѹ�
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
		CommonMath::InverseZ(out_pos);
	}
}