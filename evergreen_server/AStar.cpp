#include "pch.h"
#include "AStar.h"
#include "Navigator.h"
#include "NaviCell.h"

//float AStar::Heuristic(const NaviCell* const a, const NaviCell* const b) noexcept
//{
//    return(a->GetCellCenter() - b->GetCellCenter()).LengthSquared();
//}

float AStar::Heuristic(const NaviCell* const a, const NaviCell* const b)noexcept
{
    const auto v1 = a->GetCellCenter();
    const auto v2 = b->GetCellCenter();
    const auto dx = std::abs(v1.x - v2.x);
    const auto dy = std::abs(v1.y - v2.y);
    const auto dz = std::abs(v1.z - v2.z);

    return std::max(std::max(dx, dy), dz);
}
bool HasLineOfSight(const NaviCell* const a, const NaviCell* const b, const NaviMesh* const pNaviMesh)noexcept
{
    // a�� b ���� ���� ���̿� ��ֹ��� ������ Ȯ���ϴ� ������ ����
    // �� �Լ��� �� �� ������ ��θ� ���� raycast �Ǵ� �浹 �˻縦 ���� ���� �̵��� �������� �Ǵ���.
    return true;
}

ServerCore::Vector<DirectX::SimpleMath::Vector3> AStar::OptimizePath(const ServerCore::Vector<DirectX::SimpleMath::Vector3>& path, const NaviMesh* const pNaviMesh, const DirectX::SimpleMath::Vector3& dpos) noexcept
{
    ServerCore::Vector<DirectX::SimpleMath::Vector3> optimizedPath;
    if (path.empty()) return optimizedPath;

    // ó�� ���� ������ ����
    optimizedPath.push_back(path[0]);
    int lastAdded = 0; // ���������� �߰��� ���� �ε���

    // ��ο��� �������� �̵��� �� �ִ� ������ ã��
    for (int i = 1; i < path.size(); ++i)
    {
        break;
        // ���� ���� ���������� �߰��� �� ���̿� ���� �̵��� �������� Ȯ��
       // if (!HasLineOfSight(path[lastAdded], path[i], pNaviMesh))
        {
            // ���� �̵��� �Ұ����ϸ� ���� ���� ��ο� �߰�
            optimizedPath.push_back(path[i - 1]);
            lastAdded = i - 1; // ���������� �߰��� �� ����
        }
    }

    // ������ ������ �� �߰�
    if(optimizedPath.size()!=1)
        optimizedPath.push_back(path.back());
    else
        optimizedPath.push_back(dpos);
   // std::ranges::reverse(optimizedPath);
    return optimizedPath;
}

ServerCore::Vector<DirectX::SimpleMath::Vector3> AStar::GetAStarPath(const NaviMesh* const pNaviMesh, const int start, const int dest, const DirectX::SimpleMath::Vector3& dpos) noexcept
{
    const auto start_cell = pNaviMesh->GetNaviCell(start);
    const auto dest_cell = pNaviMesh->GetNaviCell(dest);

    const auto& cell_costs = pNaviMesh->GetCosts();

    ServerCore::PriorityQueue<NaviNode> pq;
    ServerCore::HashMap<int, float> g_val;
    ServerCore::HashMap<int, int> came_from;

    pq.emplace(start_cell->GetCellIndex(pNaviMesh), 0.f + Heuristic(start_cell, dest_cell));
    g_val[start_cell->GetCellIndex(pNaviMesh)] = 0.f;
    came_from[start_cell->GetCellIndex(pNaviMesh)] = start_cell->GetCellIndex(pNaviMesh);
    while (!pq.empty())
    {
        const auto cur_idx = pq.top().idx;
        const auto cur_cell = pNaviMesh->GetNaviCell(cur_idx);
        pq.pop();

        if (dest_cell == cur_cell)
        {
            ServerCore::Vector<DirectX::SimpleMath::Vector3> path;
            int current_idx = cur_idx;

           
            while (current_idx != start)
            {
                path.push_back(pNaviMesh->GetNaviCell(current_idx)->GetCellCenter());
                current_idx = came_from[current_idx];
            }

            path.push_back(start_cell->GetCellCenter());
            std::reverse(path.begin(), path.end()); 

          
            return OptimizePath(path, pNaviMesh,dpos);
        }

        const float cur_g = g_val[cur_idx];

        for (int i = 0; i < 3; ++i)
        {
            const auto next_node = cur_cell->GetNeighbourhood(pNaviMesh, i);
            if (next_node == nullptr) continue;
            const auto idx = next_node->GetCellIndex(pNaviMesh);
            const auto g = cur_g + cell_costs[cur_idx][i];
            if (!g_val.contains(idx) || g_val[idx] > g)
            {
                g_val[idx] = g;
                pq.emplace(idx, g + Heuristic(dest_cell, next_node));
                came_from[idx] = cur_idx;
            }
        }
    }

    return {};
}