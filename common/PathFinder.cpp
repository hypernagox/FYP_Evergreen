#include "pch.h"
#include "PathFinder.h"
#include "NavigationMesh.h"
#include "NaviAgent.h"
#include "Navigator.h"

namespace Common
{
    std::span<DirectX::SimpleMath::Vector3> PathFinder::GetPath(const DirectX::SimpleMath::Vector3& start, const DirectX::SimpleMath::Vector3& dest,int idx) const noexcept
    {
        // TODO: 매직넘버
        dtPolyRef path[10];
        int pathCount;
        const auto nav = m_agent->GetNavMesh();
        const auto nav_q = nav->GetNavMeshQuery();
        const auto nav_f = nav->GetNavFilter();
        const auto start_z_pos = CommonMath::InverseZ(start);
        auto dest_z_pos = CommonMath::InverseZ(dest);

        float t;
        float hitNormal[3];

        const auto start_poly = m_agent->GetCurCell().GetPolyRef();

       //dtStatus status = nav_q->raycast(start_poly, &start_z_pos.x, &dest_z_pos.x, nav_f, &t, hitNormal, path, &pathCount, 10);
       //
       //if (status == DT_SUCCESS && t == 1.0f)
       //{
       //    // TODO: 더 아름다운 방법으로 변경
       //    thread_local Vector3 p[1];
       //    p[0] = dest;
       //    return p;
       //}

        dtPolyRef dest_poly;


        nav_q->findNearestPoly(&dest_z_pos.x, NaviCell::g_extent, nav_f, &dest_poly, &dest_z_pos.x);
        
       // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->requestMoveTarget(idx, dest_poly, &dest_z_pos.x);

        dtStatus status = nav_q->findPath(start_poly, dest_poly, &start_z_pos.x, &dest_z_pos.x, nav_f, path, &pathCount, 10);

        if (dtStatusFailed(status))
        {
            std::cout << "못 찾음\n";
            return {};
        }

        // TODO: 매직넘버
        constinit thread_local float straightPathRaw[10 * 3] = {};
       // thread_local DirectX::SimpleMath::Vector3 straightPath[10];
        const auto straightPath = (Vector3*)straightPathRaw;
        unsigned char straightPathFlags[10];
        dtPolyRef straightPathPolys[10];
        int straightPathCount = 0;


        status = nav_q->findStraightPath(&start_z_pos.x, &dest_z_pos.x, path, pathCount, &straightPath[0].x, straightPathFlags, straightPathPolys, &straightPathCount, 10);
        if (dtStatusFailed(status))
        {
            std::cout << "못 찾음\n";
            return {};
        }

        auto b = straightPath;
        const auto e = straightPath + straightPathCount;
        while (e != b)
        {
            b->z = -b->z;
            ++b;
        }
        return { straightPath,straightPath + straightPathCount };
        // TODO: 매직넘버
        // return SmoothPath(straightPath, straightPathCount, 4);
    }
}