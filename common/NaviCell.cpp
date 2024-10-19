#include "pch.h"
#include "NaviCell.h"
#include "NavigationMesh.h"

namespace Common
{
    NaviCell::NaviCell(Vector3& pos, const NavigationMesh* const pNavMesh)noexcept
    {
        CommonMath::InverseZ(pos);
        pNavMesh->GetNavMeshQuery()->findNearestPoly(&pos.x, g_extent, pNavMesh->GetNavFilter(), &m_cell, &pos.x);
        CommonMath::InverseZ(pos);
    }

    float NaviCell::CalculateHeight(const DirectX::SimpleMath::Vector3& pos, const NavigationMesh* const pNavMesh) const noexcept
    {
        float h = 0.f;
        const auto temp = CommonMath::InverseZ(pos);
        pNavMesh->GetNavMeshQuery()->getPolyHeight(m_cell, &temp.x, &h);
        return h;
    }

    float NaviCell::GetSlopeAngle(const NavigationMesh* const pNavMesh) const noexcept
    {
        const dtMeshTile* tile = nullptr;
        const dtPoly* poly = nullptr;

        pNavMesh->GetNavMesh()->getTileAndPolyByRef(m_cell, &tile, &poly);

        if (!tile || !poly)return{};

        const float* const v0 = &tile->verts[poly->verts[0] * 3];
        const float* const v1 = &tile->verts[poly->verts[1] * 3];
        const float* const v2 = &tile->verts[poly->verts[2] * 3];

        const DirectX::SimpleMath::Vector3 vec1(v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]);
        const DirectX::SimpleMath::Vector3 vec2(v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]);
        const DirectX::SimpleMath::Vector3 normal = CommonMath::Normalized(vec1.Cross(vec2));

        return std::acos(normal.Dot(DirectX::SimpleMath::Vector3{ 0.0f, 1.0f, 0.0f }));
    }
}