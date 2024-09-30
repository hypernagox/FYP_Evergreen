#pragma once
#include "pch.h"

class NaviCell
{
    friend class NaviMesh;

    static constexpr const float NAVI_EPSILON = FLT_EPSILON;

    static DirectX::SimpleMath::Plane CreateNormalizedPlane(const DirectX::SimpleMath::Vector3& v1, const DirectX::SimpleMath::Vector3& v2, const DirectX::SimpleMath::Vector3& v3) noexcept {
        DirectX::SimpleMath::Plane plane{ v1,v2,v3 };
        plane.Normalize();
        return plane;
    }
public:
    NaviCell(const DirectX::SimpleMath::Vector3& v1, const DirectX::SimpleMath::Vector3& v2, const DirectX::SimpleMath::Vector3& v3)
        :
        m_vertices{ v1, v2, v3 },
        m_plane{ CreateNormalizedPlane(v1, v2, v3) },
        m_centroid{ (v1 + v2 + v3) / 3.0f },
        m_mid{ (v1 + v2) / 2.0f, (v2 + v3) / 2.0f, (v3 + v1) / 2.0f },
        m_arrivalCost{ (m_mid[0] - m_centroid).Length(), (m_mid[1] - m_centroid).Length(), (m_mid[2] - m_centroid).Length() }
    {
        if (std::abs(m_plane.Normal().y) < NAVI_EPSILON)
        {
            throw std::runtime_error("Cannot calculate Y because the plane is vertical.");
        }
    }

    float CalculateY(const float x, const float z) const noexcept
    {
        const auto abc = m_plane.Normal();
        return (-abc.x * x - abc.z * z - m_plane.D()) / abc.y;
    }

    float CalculateSelfSlopeAngle() const noexcept
    {
        constexpr const DirectX::SimpleMath::Vector3 upVector = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
        return acosf(m_plane.Normal().Dot(upVector));
    }

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
       
        return cross1.Dot(cross2) >= 0.0f && cross1.Dot(cross3) >= 0.0f;
    }


    float CalculateSlopeAngleDifference(const NaviCell& otherCell) const noexcept
    {
        const float dotProduct = m_plane.Normal().Dot(otherCell.m_plane.Normal());
        const float cosTheta = std::clamp(dotProduct, -1.0f, 1.0f);
        return acosf(cosTheta);
    }

    float CalculateSlopeAngleDifference(const NaviCell* const otherCell) const noexcept
    {
        return CalculateSlopeAngleDifference(*otherCell);
    }

    static bool AreEdgesEqual(const DirectX::SimpleMath::Vector3& v1, const DirectX::SimpleMath::Vector3& v2, const DirectX::SimpleMath::Vector3& w1, const DirectX::SimpleMath::Vector3& w2) noexcept {
        constexpr const float EDGE_COMPARISON_EPSILON = 1e-5f;
        return ((v1 - w1).LengthSquared() < EDGE_COMPARISON_EPSILON && (v2 - w2).LengthSquared() < EDGE_COMPARISON_EPSILON) ||
            ((v1 - w2).LengthSquared() < EDGE_COMPARISON_EPSILON && (v2 - w1).LengthSquared() < EDGE_COMPARISON_EPSILON);
    }

private:
    const DirectX::SimpleMath::Plane m_plane;
    const DirectX::SimpleMath::Vector3 m_vertices[3];
    const DirectX::SimpleMath::Vector3 m_centroid;
    const DirectX::SimpleMath::Vector3 m_mid[3];
    const float m_arrivalCost[3];
    const NaviCell* m_links[3] = {};
};

