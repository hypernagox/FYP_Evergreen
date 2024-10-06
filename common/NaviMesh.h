#pragma once
#include "pch.h"

class NaviCell;

class NaviMesh
{
public:
    // 이 섹션에는 셀 자체를 완전 탐색하는 녀석들임

    // 정확히 이점이 반드시 삼각형안에 존재하나?
    const NaviCell* FindCellContainingPoint(const DirectX::SimpleMath::Vector3& point) const noexcept;

    // 그나마 가까운거
    const NaviCell* FindCellContainingOrClosestPoint(const DirectX::SimpleMath::Vector3& point) const noexcept;
    const NaviCell* FindCellWithClosestCenter(const DirectX::SimpleMath::Vector3& point) const noexcept;

    const NaviCell* FindRayIntersectingCell(const DirectX::SimpleMath::Vector3& point) const noexcept;

public:
    const NaviCell* FindRayIntersectingCellInNeighbourhoods(
        const NaviCell* const cell,
        const DirectX::SimpleMath::Vector3& point,
        const int depth
    )const noexcept;

public:
    // 지금 셀에서 매개변수로 받은 깊이만큼 이웃셀을 탐색해서 그 목록을 반환.
    std::vector<const NaviCell*> GetAdjacentCells(const NaviCell* const cell, int depth)const noexcept;
    const NaviCell* GetNaviCell(const int idx)const noexcept;
    int GetNaviCellIndex(const NaviCell* const cell)const noexcept;
    const auto& GetCosts()const noexcept { return m_neighbourhoodDists; }
private:
    void Bake(const std::vector<DirectX::SimpleMath::Vector3>& vertices, const std::vector<UINT>& indices)noexcept;
public:
    void Save(const std::wstring_view path)const;
    void LoadByObj(const std::wstring_view objFilePath);
    void Load(const std::wstring_view path);
private:
    // 이웃의 정의: 나랑 변을 공유하는 삼각형(셀)들 (최대 3개)
    using NeighbourhoodDists = std::array<float, 3>;

    std::vector<NaviCell> m_cells;

    // 길 찾기 할 때 쓸 것
    // 예) 3번 인덱스의 셀에서 1번 이웃과의 거리는 ? -> m_neighbourhoodDists[3][1]
    // 이웃이 없다면 매우 큰 값으로 세팅 되어있음 (애초에 이웃없으면 nullptr)
    std::vector<NeighbourhoodDists> m_neighbourhoodDists;
};
