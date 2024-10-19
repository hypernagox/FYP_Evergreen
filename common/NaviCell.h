#pragma once
#include "pch.h"

namespace Common
{
    class NavigationMesh;

    class NaviCell
    {
        friend class NavigationMesh;
    public:
        static constexpr float g_extent[3]{ 10,10,10 };
    public:
        NaviCell()noexcept = default;
        NaviCell(Vector3& pos, const NavigationMesh* const pNavMesh)noexcept;
        NaviCell(const dtPolyRef cell)noexcept :m_cell{ cell } {}
    public:
        float CalculateHeight(const DirectX::SimpleMath::Vector3& pos, const NavigationMesh* const pNavMesh)const noexcept;
        float GetSlopeAngle(const NavigationMesh* const pNavMesh) const noexcept;
        const auto GetPolyRef()const noexcept { return m_cell; }
        void SetPolyRef(const dtPolyRef cell)noexcept { m_cell = cell; }
    private:
        dtPolyRef m_cell;
    };
}