#pragma once
#include "pch.h"

class NaviCell;

class NaviMesh
{
public:
    // �� ���ǿ��� �� ��ü�� ���� Ž���ϴ� �༮����

    // ��Ȯ�� ������ �ݵ�� �ﰢ���ȿ� �����ϳ�?
    const NaviCell* FindCellContainingPoint(const DirectX::SimpleMath::Vector3& point) const noexcept;

    // �׳��� ������
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
    // ���� ������ �Ű������� ���� ���̸�ŭ �̿����� Ž���ؼ� �� ����� ��ȯ.
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
    // �̿��� ����: ���� ���� �����ϴ� �ﰢ��(��)�� (�ִ� 3��)
    using NeighbourhoodDists = std::array<float, 3>;

    std::vector<NaviCell> m_cells;

    // �� ã�� �� �� �� ��
    // ��) 3�� �ε����� ������ 1�� �̿����� �Ÿ��� ? -> m_neighbourhoodDists[3][1]
    // �̿��� ���ٸ� �ſ� ū ������ ���� �Ǿ����� (���ʿ� �̿������� nullptr)
    std::vector<NeighbourhoodDists> m_neighbourhoodDists;
};
