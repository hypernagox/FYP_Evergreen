#pragma once
#include "pch.h"

class NaviCell
{
    friend class NaviMesh;

    static DirectX::SimpleMath::Plane CreateNormalizedPlane(const DirectX::SimpleMath::Vector3& v1, const DirectX::SimpleMath::Vector3& v2, const DirectX::SimpleMath::Vector3& v3) noexcept {
        DirectX::SimpleMath::Plane plane{ v1,v2,v3 };
        plane.Normalize();
        return plane;
    }
public:
    using Neighbourhoods = std::array<const NaviCell*, 3>;

    NaviCell()noexcept = default;
    NaviCell(const DirectX::SimpleMath::Vector3& v1, const DirectX::SimpleMath::Vector3& v2, const DirectX::SimpleMath::Vector3& v3)noexcept
        : m_vertices{ v1, v2, v3 }
        , m_plane{ CreateNormalizedPlane(v1, v2, v3) }
    {
    }

    // 평면의 방정식을 이용해서 Y위치를 보정한다
    float CalculateY(const float x, const float z) const noexcept
    {
        static constexpr const float NAVI_EPSILON = 1e-6f;

        const auto abc = m_plane.Normal();
        if (std::abs(abc.y) <= NAVI_EPSILON)return GetCellCenter().y;
        return -(-abc.x * x - abc.z * z - m_plane.D()) / abc.y;
    }

    // 내 경사각
    float CalculateSelfSlopeAngle() const noexcept
    {
        constexpr const DirectX::SimpleMath::Vector3 upVector = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
        return acosf(m_plane.Normal().Dot(upVector));
    }

    // 이 점이 셀에 속하나?
    bool ContainsPoint(const DirectX::SimpleMath::Vector3& point) const noexcept
    {
        const DirectX::SimpleMath::Vector3 AB = m_vertices[1] - m_vertices[0];
        const DirectX::SimpleMath::Vector3 BC = m_vertices[2] - m_vertices[1];
        const DirectX::SimpleMath::Vector3 CA = m_vertices[0] - m_vertices[2];

        const DirectX::SimpleMath::Vector3 AP = point - m_vertices[0];
        const DirectX::SimpleMath::Vector3 BP = point - m_vertices[1];
        const DirectX::SimpleMath::Vector3 CP = point - m_vertices[2];

        const DirectX::SimpleMath::Vector3 cross1 = AB.Cross(AP);
        const DirectX::SimpleMath::Vector3 cross2 = BC.Cross(BP);
        const DirectX::SimpleMath::Vector3 cross3 = CA.Cross(CP);
       
        return cross1.Dot(cross2) * cross1.Dot(cross3) >= 0.f;
    }

    // 다른셀과 나의 경사각
    float CalculateSlopeAngleDifference(const NaviCell* const otherCell) const noexcept
    {
        const float dotProduct = m_plane.Normal().Dot(otherCell->m_plane.Normal());
        const float cosTheta = std::clamp(dotProduct, -1.0f, 1.0f);
        return acosf(cosTheta);
    }

    bool RayIntersectsCell(const DirectX::SimpleMath::Vector3& point, float& t) const noexcept
    {
        // 원랜 위에서 밑으로만 쏘는데 지금 바닥에 박혀서 모든 폴리곤이 내 머리위에 있는 경우에 대해서
        // 이거도 나중에좀 분리를할듯 2방향으로 광선을 쏨
        constexpr const DirectX::XMVECTOR direction = { 0.f,-1.f,0.f };
        constexpr const DirectX::XMVECTOR direction2 = { 0.f,1.f,0.f };
        const DirectX::XMVECTOR origin = DirectX::XMLoadFloat3(&point);
        const DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&m_vertices[0]);
        const DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&m_vertices[1]);
        const DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&m_vertices[2]);
       
        return DirectX::TriangleTests::Intersects(origin, direction, v0, v1, v2, t)
            || DirectX::TriangleTests::Intersects(origin, direction2, v0, v1, v2, t);
    }

    DirectX::SimpleMath::Vector3 GetCellCenter()const noexcept { return (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.f; }

    // 나는 몇번 인덱스인가?
    int GetCellIndex(const NaviMesh* const curNaviMesh)const noexcept;
    
    const NaviCell* GetNeighbourhood(const NaviMesh* const curNaviMesh,const int idx)const noexcept;

    Neighbourhoods GetNeighbourhoods(const NaviMesh* const curNaviMesh)const noexcept
    {
        const NaviCell* cells[3];
        for (int i = 0; i < 3; ++i)cells[i] = GetNeighbourhood(curNaviMesh, i);
        return std::to_array(cells);
    }
private:
    DirectX::SimpleMath::Plane m_plane;
    DirectX::SimpleMath::Vector3 m_vertices[3];
    int m_neighbourhoodsIdx[3] = { -1,-1,-1 };
    // 내 이웃 셀의 네비메시 상에서의 인덱스
    // 이웃이 없다면 -1임
};

