#pragma once
#include "pch.h"
#include "NaviCell.h"

class NaviMesh
{
    static std::vector<NaviCell> LoadObjToNaviCellsWithLinks(const std::wstring_view objFilePath)
    {
        std::wifstream file{ objFilePath.data() };
        if (!file)
        {
            throw std::runtime_error("Unable to open .obj file");
        }

        std::vector<DirectX::SimpleMath::Vector3> vertices;
        std::vector<NaviCell> navCells;

        std::wstring line;
        while (std::getline(file, line)) {
            std::wistringstream iss(line);
            std::wstring prefix;
            iss >> prefix;

            if (prefix == L"v")
            {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            }
            else if (prefix == L"f")
            {
                int v1, v2, v3;
                iss >> v1 >> v2 >> v3;
                navCells.emplace_back(vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1]);
            }
        }

        for (size_t i = 0; i < navCells.size(); ++i)
        {
            for (size_t j = i + 1; j < navCells.size(); ++j)
            {
                for (int side_i = 0; side_i < 3; ++side_i)
                {
                    for (int side_j = 0; side_j < 3; ++side_j)
                    {
                        if (NaviCell::AreEdgesEqual(navCells[i].m_vertices[side_i], navCells[i].m_vertices[(side_i + 1) % 3],
                            navCells[j].m_vertices[side_j], navCells[j].m_vertices[(side_j + 1) % 3])) {
                            navCells[i].m_links[side_i] = &navCells[j];
                            navCells[j].m_links[side_j] = &navCells[i];
                        }
                    }
                }
            }
        }

        return navCells;
    }
public:
    const std::vector<NaviCell> m_cells;

    NaviMesh(const std::wstring_view objFilePath)
        : m_cells{ LoadObjToNaviCellsWithLinks(objFilePath) }
    {
    }

    const NaviCell* FindCellContainingPoint(const DirectX::SimpleMath::Vector3& point) const noexcept
    {
        auto b = m_cells.data();
        const auto e = b + m_cells.size();
        while (e != b)
        {
            if (b->ContainsPoint(point))
            {
                return b;
            }
            ++b;
        }
        return nullptr;
    }

    const NaviCell* FindCellContainingOrClosestPoint(const DirectX::SimpleMath::Vector3& point) const noexcept
    {
        const NaviCell* closestCell = nullptr;
        float closestDistanceSquared = std::numeric_limits<float>::max();

        auto b = m_cells.data();
        const auto e = b + m_cells.size();

        while (e != b)
        {
            if (b->ContainsPoint(point))
            {
                return b;
            }

            const float distanceSquared = (b->m_centroid - point).LengthSquared();

            if (distanceSquared < closestDistanceSquared)
            {
                closestDistanceSquared = distanceSquared;
                closestCell = b;
            }

            ++b;
        }
        return closestCell;
    }
};
