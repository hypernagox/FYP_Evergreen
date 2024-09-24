#include "pch.h"
#include "ContentsBehaviorNode.h"
#include "TickTimer.h"
#include "Session.h"
#include "AStar.h"
using namespace ServerCore;

static NaviMesh& GetNavMesh(std::string_view fileName)
{
    static NaviMesh navMesh(50.0f);
    navMesh.LoadObj(fileName.data());
    return navMesh;
}
NodeStatus MoveNode::Tick(const ComponentSystemEX* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    //const auto cur_pos = pOwnerEntity->GetComp<PositionComponent>()->pos;
    ////if (!m_bHasDest)
    //{
    //    const auto awaker = pOwnerEntity->GetIocpComponent<TickTimerBT>()->GetAwaker();
    //    if (!awaker)return NodeStatus::FAILURE;
    //    m_bHasDest = true;
    //
    //    dest_pos = awaker->GetComp<PositionComponent>()->pos;
    //
    //    const auto dx = dest_pos.x - cur_pos.x;
    //    const auto dy = dest_pos.y - cur_pos.y;
    //    const auto dz = dest_pos.z - cur_pos.z;
    //
    //    dir = Vec3{ dx,dy,dz };
    //    
    //    if (30 * 30 <= dx * dx + dy * dy + dz * dz)
    //        return NodeStatus::FAILURE;
    //}
    //
    //const auto dx = cur_pos.x + dir.x*.1f * .5f;
    //const auto dy = cur_pos.y + dir.y*.1f * .5f;
    //const auto dz = cur_pos.z + dir.z*.1f * .5f;
    //
    //pOwnerEntity->GetComp<PositionComponent>()->pos = Vec3{ dx,dy,dz };
    //
    //
    return NodeStatus::SUCCESS;
}

NodeStatus RangeCheckNode::Tick(const ComponentSystemEX* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;
    auto& awaker_ = bt_root_timer->GetMutableAwaker();
    uint32_t min_dist = 999999;
   
    if (m_bReEvaluate)
    {
        const auto pOwnerEntity = owner_comp_sys->GetOwnerEntity();
        const auto prev_awaker = awaker_.get();
        ContentsEntity* target = nullptr;
        m_bReEvaluate = false;

        const auto session_list = pOwnerEntity->GetCurSector()->GetSessionCopyListIncRef();
        for (const auto s : session_list)
        {
           const auto pos  = s->GetComp<PositionComponent>()->pos;
           const auto dx = pos.x - cur_pos.x;
           const auto dy = pos.y - cur_pos.y;
           const auto dz = pos.z - cur_pos.z;
           
           const uint32_t dist = (int)(dx * dx + dy * dy + dz * dz);
           if (min_dist > dist)
           {
               min_dist = dist;
               target = s;
           }
           s->DecRef();
        }
        if (target)
        {
            awaker_ = target->SharedFromThis();
            if(m_range * m_range > min_dist && prev_awaker != awaker_.get())
                awaker_->GetSession()->SendAsync(Create_s2c_MONSTER_AGGRO_START((Nagox::Enum::GROUP_TYPE)pOwnerEntity->GetObjectType(), pOwnerEntity->GetObjectTypeInfo()));
        }
    }
    

    
    if (!awaker_->IsValid())return NodeStatus::FAILURE;
    
    const auto dest_pos = awaker_->GetComp<PositionComponent>()->pos;

    const auto dx = dest_pos.x - cur_pos.x;
    const auto dy = dest_pos.y - cur_pos.y;
    const auto dz = dest_pos.z - cur_pos.z;

    
    if (m_range * m_range <= dx * dx + dy * dy + dz * dz) 
    {
        return NodeStatus::FAILURE;
    }
    
    return NodeStatus::SUCCESS;
}

NodeStatus AttackNode::Tick(const ComponentSystemEX* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;

    if (!awaker)return NodeStatus::FAILURE;

    const auto dest_pos = awaker->GetComp<PositionComponent>()->pos;

    const auto dx = dest_pos.x - cur_pos.x;
    const auto dy = dest_pos.y - cur_pos.y;
    const auto dz = dest_pos.z - cur_pos.z;

   // std::cout << "공격!" << std::endl;

    if (m_attack_range * m_attack_range <= dx * dx + dy * dy + dz * dz)
        return NodeStatus::SUCCESS;

    m_accTime -= bt_root_timer->GetBTTimerDT();
    if (0.f >= m_accTime)
    {
        awaker->GetSession()->SendAsync(Create_s2c_MONSTER_ATTACK(10));

        m_accTime = 1.5f;
    }

    return NodeStatus::RUNNING;
}
Vertex GetMoveTargetPosition(NaviCell* currentCell, NaviCell* nextCell, const Vertex& origin, const Vertex& dest) {
    // 1. 두 셀이 공유하는 변이 있는지 확인
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            // 두 셀이 공유하는 변이 있으면 그 변의 중점을 반환
            if ((currentCell->v1.x == nextCell->v1.x && currentCell->v1.z == nextCell->v1.z && currentCell->v2.x == nextCell->v2.x && currentCell->v2.z == nextCell->v2.z) ||
                (currentCell->v2.x == nextCell->v2.x && currentCell->v2.z == nextCell->v2.z && currentCell->v3.x == nextCell->v3.x && currentCell->v3.z == nextCell->v3.z) ||
                (currentCell->v3.x == nextCell->v3.x && currentCell->v3.z == nextCell->v3.z && currentCell->v1.x == nextCell->v1.x && currentCell->v1.z == nextCell->v1.z)) {
                return currentCell->mid[i];  // 공유된 변의 중점(mid)을 반환
            }
        }
    }

    // 2. 공유된 변이 없는 경우, 현재 셀의 이웃들 중 가장 적당한 이동 위치 찾기
    float minDistance = std::numeric_limits<float>::infinity();  // 최소 거리 값을 매우 큰 값으로 초기화
    Vertex bestMidPoint = dest;  // 기본값으로 origin 위치

    for (int i = 0; i < 3; ++i) {
        NaviCell* neighbor = currentCell->link[i];
        if (neighbor != nullptr) {
            float distance = CalculateDistance(currentCell->center, neighbor->center);

            // 적당한 이웃: 거리가 최소인 이웃을 선택
            if (distance < minDistance) {
                minDistance = distance;
                bestMidPoint = currentCell->mid[i];  // 그 변의 중점을 반환
            }
        }
    }

    // 3. 가장 적당한 이웃이 없으면 origin 반환
    return bestMidPoint;
}


NodeStatus ChaseNode::Tick(const ComponentSystemEX* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;
    const auto pOwnerEntity = bt_root_timer->GetOwnerEntity();
    if (!awaker)return NodeStatus::FAILURE;
    
   // std::cout << cur_pos.x << ' ' << cur_pos.y << ' ' << cur_pos.z << '\n';
    
    const auto dest_pos = awaker->GetComp<PositionComponent>()->pos;
    static NaviMesh& nav = GetNavMesh("ExportedNavMesh.obj");
   // const auto  n  = nav.FindCellForPoint({ cur_pos.x / 1,cur_pos.y / 1,cur_pos.z / 1 });
    //std::cout << n->center.x << ' ' << n->center.y << ' ' << n->center.z << '\n';
    const auto path = AStar(nav.FindCellForPoint({ cur_pos.x,cur_pos.y,cur_pos.z}), nav.FindCellForPoint({ dest_pos.x,dest_pos.y,dest_pos.z }));

    if (path.size() >= 2) {
        const auto vvv = GetMoveTargetPosition(path[0], path[1], { cur_pos.x,cur_pos.y,cur_pos.z }
        , { dest_pos.x,dest_pos.y,dest_pos.z });
        owner_comp_sys->GetComp<PositionComponent>()->pos = { vvv.x,vvv.y,vvv.z };
        std::cout << path.size() << std::endl;
        return NodeStatus::RUNNING;
    }
    else if (path.size() == 1) {
       // owner_comp_sys->GetComp<PositionComponent>()->pos = dest_pos;
      //  std::cout << path.size() << std::endl;
    }
    else {
        return NodeStatus::FAILURE;
    }
    const auto dx = dest_pos.x - cur_pos.x;
    const auto dy = dest_pos.y - cur_pos.y;
    const auto dz = dest_pos.z - cur_pos.z;
    
    auto dir = DirectX::SimpleMath::Vector3{ dx,dy,dz };
    dir.Normalize();
    
    if (10 * 10 <= dx * dx + dy * dy + dz * dz) 
    {
        awaker->GetSession()->SendAsync(Create_s2c_MONSTER_AGGRO_END((Nagox::Enum::GROUP_TYPE)pOwnerEntity->GetObjectType(), pOwnerEntity->GetObjectTypeInfo()));
        return NodeStatus::FAILURE;
    }
    //if (2 * 2 >= dx * dx + dy * dy + dz * dz) {
    //    std::cout << "추격 성공" << std::endl;
    //    return NodeStatus::SUCCESS;
    //}
    const float dt_ = bt_root_timer->GetBTTimerDT();
    const auto dx2 = cur_pos.x + dir.x * 3.2f * dt_;
    const auto dy2 = cur_pos.y + dir.y * 3.2f * dt_;
    const auto dz2 = cur_pos.z + dir.z * 3.2f * dt_;

    pOwnerEntity->GetComp<PositionComponent>()->pos = Vec3{ dx2,dy2,dz2 };

    ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
    const int sector_state = pOwnerEntity->BroadcastMove(0, 0, std::move(sectors));

    if (SECTOR_STATE::USER_EMPTY & sector_state) {
        return NodeStatus::FAILURE;
    }

   // if (10 * 10 <= dx * dx + dy * dy + dz * dz) {
   //     std::cout << "추격 성공" << std::endl;
   //     return NodeStatus::SUCCESS;
   // }
    if (2 * 2 >= dx * dx + dy * dy + dz * dz) {
        std::cout << "추격 성공" << std::endl;
        return NodeStatus::SUCCESS;
    }
    return NodeStatus::RUNNING;
}

NodeStatus PatrolNode::Tick(const ComponentSystemEX* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    owner_comp_sys->GetComp<PositionComponent>()->body_angle += 1000.f * bt_root_timer->GetBTTimerDT();
    const auto pOwnerEntity = owner_comp_sys->GetOwnerEntity();
    ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
    const int sector_state = pOwnerEntity->BroadcastMove(0, 0, std::move(sectors));

    if (SECTOR_STATE::USER_EMPTY & sector_state)
        return NodeStatus::FAILURE;

    return NodeStatus::SUCCESS;
}
