#include "pch.h"
#include "PathFinder.h"
#include "NavigationMesh.h"
#include "NaviAgent.h"
#include "Navigator.h"

thread_local std::unordered_map<uint64_t, std::pair<ZeroInt, std::array<dtPolyRef, 10>>> tl_poly_vec;

namespace Common
{
    std::span<DirectX::SimpleMath::Vector3> PathFinder::GetPath(const DirectX::SimpleMath::Vector3& start, const DirectX::SimpleMath::Vector3& dest) const noexcept
    {
        constexpr const uint8_t PATH_COUNT = 10;
        int pathCount;
        const auto nav = m_agent->GetNavMesh();
        const auto nav_q = nav->GetNavMeshQuery();
        const auto nav_f = nav->GetNavFilter();
        const auto start_z_pos = CommonMath::InverseZ(start);
        auto dest_z_pos = CommonMath::InverseZ(dest);

        //float t;
        //float hitNormal[3];

       const  auto start_poly = m_agent->GetCurCell().GetPolyRef();

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


        dtStatus status =  nav_q->findNearestPoly(&dest_z_pos.x, NaviCell::g_extent, nav_f, &dest_poly, &dest_z_pos.x);
        
        if (dtStatusFailed(status))
        {
            // std::cout << "못 찾음\n";
            return {};
        }
       // NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->requestMoveTarget(idx, dest_poly, &dest_z_pos.x);
       extern thread_local std::unordered_map<uint64_t, std::pair<ZeroInt, std::array<dtPolyRef, 10>>> tl_poly_vec;
       uint64_t start_poly64 = (uint64_t)start_poly;
       uint64_t dest_poly64 = (uint64_t)dest_poly;
       if (start_poly64 > dest_poly64)std::swap(start_poly64, dest_poly64);
       const uint64_t key = (start_poly64 << 32) | dest_poly64;
       auto& num_path = tl_poly_vec[key];
       auto& path = num_path.second;
       if (0 == num_path.first)
       {
           status = nav_q->findPath(start_poly, dest_poly, &start_z_pos.x, &dest_z_pos.x, nav_f, path.data(), &pathCount, PATH_COUNT);
           if (dtStatusFailed(status))
           {
               // std::cout << "못 찾음\n";
               return {};
           }
           num_path.first.num = pathCount;
       }
       else
       {
           pathCount = num_path.first;
       }

        // TODO: 매직넘버
        constinit thread_local float straightPathRaw[PATH_COUNT * 3] = {};

       // thread_local DirectX::SimpleMath::Vector3 straightPath[10];
        const auto straightPath = (Vector3*)straightPathRaw;
        unsigned char straightPathFlags[PATH_COUNT];
        dtPolyRef straightPathPolys[PATH_COUNT];
        int straightPathCount = 0;


        status = nav_q->findStraightPath(&start_z_pos.x, &dest_z_pos.x, path.data(), pathCount, &straightPath[0].x, straightPathFlags, straightPathPolys, &straightPathCount, PATH_COUNT);
        if (dtStatusFailed(status))
        {
            //std::cout << "못 찾음\n";
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
    }
}