#include "pch.h"
#include "NaviMesh.h"
#include "NaviCell.h"

void NaviMesh::Bake(const std::vector<DirectX::SimpleMath::Vector3>& vertices, const std::vector<UINT>& indices) noexcept
{
    auto& navCells = this->m_cells;

    const auto num_of_indices = (int)indices.size();

    constexpr const float sentinel_dist[3]{ std::numeric_limits<float>::max() / 2.f
        ,std::numeric_limits<float>::max() / 2.f ,std::numeric_limits<float>::max() / 2.f }; // AStar 돌릴 때 가중치 오버플로 방지용 /2 하였음

    for (int i = 0; i < num_of_indices; i += 3)
    {
        navCells.emplace_back(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
    }

    const auto num_of_cells = (int)navCells.size();
    m_neighbourhoodDists.resize(num_of_cells,std::to_array(sentinel_dist));

    for (int i = 0; i < num_of_cells; ++i)
    {
        NaviCell& currentCell = navCells[i];

        const auto currentCentroid = currentCell.GetCellCenter();
     
        for (int j = i + 1; j < num_of_cells; ++j)
        {
            NaviCell& otherCell = navCells[j];

            for (int side_i = 0; side_i < 3; ++side_i)
            {
                if (currentCell.m_neighbourhoodsIdx[side_i] != -1) continue;  

                for (int side_j = 0; side_j < 3; ++side_j)
                {
                    if ((currentCell.m_vertices[side_i] == otherCell.m_vertices[side_j] &&
                        currentCell.m_vertices[(side_i + 1) % 3] == otherCell.m_vertices[(side_j + 1) % 3]) ||
                        (currentCell.m_vertices[side_i] == otherCell.m_vertices[(side_j + 1) % 3] &&
                            currentCell.m_vertices[(side_i + 1) % 3] == otherCell.m_vertices[side_j]))
                    {
                        currentCell.m_neighbourhoodsIdx[side_i] = j;
                        otherCell.m_neighbourhoodsIdx[side_j] = i;

                        const auto sharedEdgeMidpoint = (currentCell.m_vertices[side_i] + currentCell.m_vertices[(side_i + 1) % 3]) / 2.0f;

                        const float distance = (currentCentroid - sharedEdgeMidpoint).Length();

                        m_neighbourhoodDists[i][side_i] = distance;

                        const auto otherCentroid = otherCell.GetCellCenter();

                        m_neighbourhoodDists[j][side_j] = (otherCentroid - sharedEdgeMidpoint).Length();

                        break;  
                    }
                }
            }
        }
    }
}

void NaviMesh::Save(const std::wstring_view path) const
{
    std::ofstream out{ path.data(),std::ios::binary };
    const int num_of_cells = (int)m_cells.size();
    out.write((const char*)&num_of_cells, sizeof(num_of_cells));
    out.write((const char*)m_cells.data(), sizeof(NaviCell) * num_of_cells);
    out.write((const char*)m_neighbourhoodDists.data(), sizeof(NeighbourhoodDists) * num_of_cells);
}

void NaviMesh::LoadByObj(const std::wstring_view objFilePath)
{
    std::wifstream file{ objFilePath.data() };
    if (!file)
    {
        throw std::runtime_error{ "Cannot Open File !" };
    }

    std::vector<DirectX::SimpleMath::Vector3> vertices;
    std::vector<UINT> indices;

    std::wstring line;
    while (std::getline(file, line))
    {
        std::wistringstream iss(line);
        std::wstring prefix;
        iss >> prefix;

        if (prefix == L"v")
        {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.emplace_back(x, -y, -z);
        }
        else if (prefix == L"f")
        {
            std::vector<int> faceIndices;
            std::wstring vertexStr;

            while (iss >> vertexStr)
            {
                int vertexIndex = std::stoi(vertexStr.substr(0, vertexStr.find(L'/')));
                faceIndices.push_back(vertexIndex - 1);
            }

            if (faceIndices.size() == 3)
            {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[1]);
                indices.push_back(faceIndices[2]);
            }
            else if (faceIndices.size() >= 4)
            {
                for (size_t i = 1; i < faceIndices.size() - 1; ++i)
                {
                    indices.push_back(faceIndices[0]);
                    indices.push_back(faceIndices[i]);
                    indices.push_back(faceIndices[i + 1]);
                }
            }
        }
    }

    Bake(vertices, indices);
}

void NaviMesh::Load(const std::wstring_view path) 
{
    std::ifstream in{ path.data(),std::ios::binary };
    int num_of_cells = 0;
    in.read((char*)&num_of_cells, sizeof(num_of_cells));
    if (0 == num_of_cells)
    {
        throw std::runtime_error{ "Cannot Read Bin" };
    }
    m_cells.resize(num_of_cells);
    m_neighbourhoodDists.resize(num_of_cells);
    in.read((char*)m_cells.data(), sizeof(NaviCell) * num_of_cells);
    in.read((char*)m_neighbourhoodDists.data(), sizeof(NeighbourhoodDists) * num_of_cells);
}

const NaviCell* NaviMesh::FindCellContainingPoint(const DirectX::SimpleMath::Vector3& point) const noexcept
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

const NaviCell* NaviMesh::FindCellContainingOrClosestPoint(const DirectX::SimpleMath::Vector3& point) const noexcept
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
        const float distanceSquared = (b->GetCellCenter() - point).LengthSquared();

        if (distanceSquared < closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            closestCell = b;
        }
        ++b;
    }
    return closestCell;
}

const NaviCell* NaviMesh::FindCellWithClosestCenter(const DirectX::SimpleMath::Vector3& point) const noexcept
{
    const NaviCell* closestCell = nullptr;
    float closestDistanceSquared = std::numeric_limits<float>::max();

    auto b = m_cells.data();
    const auto e = b + m_cells.size();

    while (e != b)
    {
        const float distanceSquared = (b->GetCellCenter() - point).LengthSquared();

        if (distanceSquared < closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            closestCell = b;
        }
        ++b;
    }

    return closestCell;
}

const NaviCell* NaviMesh::FindRayIntersectingCell(const DirectX::SimpleMath::Vector3& point) const noexcept
{
    const NaviCell* firstIntersectedCell = nullptr;
    float minT = std::numeric_limits<float>::max();

    auto b = m_cells.data();
    const auto e = b + m_cells.size();
    while (e != b)
    {
        float t;
        if (b->RayIntersectsCell(point, t))
        {
            if (t < minT)
            {
                minT = t;
                firstIntersectedCell = b;
            }
        }
        ++b;
    }

    return firstIntersectedCell;
}

const NaviCell* NaviMesh::FindRayIntersectingCellInNeighbourhoods(const NaviCell* const cell, const DirectX::SimpleMath::Vector3& point, const int depth) const noexcept
{
    // 밑에꺼랑 중복 스멜이긴 한데, 앞으로 보정을 어떻게하냐에 따라 일단 냅둠
    // DFS 탐색으로 적당한 깊이까지 이웃셀을 조사해본다.
    // 상용 프로덕트에서 리커전은 죄악
    const NaviCell* firstIntersectedCell = nullptr;
    float minT = std::numeric_limits<float>::max();

    // 자주 쓸 녀석인데 메모리 할당하고 함수 끝나면 해제하고 반복하는게 서버입장에선 오버헤드
    thread_local std::unordered_set<const NaviCell*> s;
    thread_local std::vector<std::pair<const NaviCell*,int>> v;
    v.emplace_back(cell, 0);
    while (!v.empty())
    {
        const auto [cur_cell, cur_depth] = v.back();
        v.pop_back();
        if (depth > cur_depth)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (const auto n = cur_cell->GetNeighbourhood(this, i))
                {
                    if (true == s.emplace(n).second)
                        v.emplace_back(n, cur_depth + 1);
                }
            }
        }
        float t;
        if (cur_cell->RayIntersectsCell(point, t))
        {
            if (t < minT)
            {
                minT = t;
                firstIntersectedCell = cur_cell;
            }
        }
    }
    s.clear(); v.clear();
    return firstIntersectedCell;
}

std::vector<const NaviCell*> NaviMesh::GetAdjacentCells(const NaviCell* const cell,const int depth) const noexcept
{
    // 윗코드랑 중복같긴한데 일단 셀 찾기 정책이 어떻게 될지 모르니 냅둠
    thread_local std::unordered_map<const NaviCell*, int> visited;
    std::vector<const NaviCell*> v;
    v.emplace_back(cell);
    visited.emplace(cell, 0);
    while (!v.empty())
    {
        const auto cur_cell = v.back();
        const auto cur_depth = visited[cur_cell] + 1;
        v.pop_back();
        if (depth >= cur_depth)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (const auto n = cur_cell->GetNeighbourhood(this, i))
                {
                    if (true == visited.emplace(n, cur_depth).second)
                        v.emplace_back(n);
                }
            }
        }
    }
    visited.clear();
    return v;
}

const NaviCell* NaviMesh::GetNaviCell(const int idx) const noexcept
{
    return m_cells.data() + idx;
}

int NaviMesh::GetNaviCellIndex(const NaviCell* const cell) const noexcept
{
    return static_cast<int>(cell - m_cells.data());
}
