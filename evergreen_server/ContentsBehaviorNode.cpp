#include "pch.h"
#include "ContentsBehaviorNode.h"
#include "TickTimer.h"
#include "Session.h"
#include "AStar.h"
#include "Navigator.h"
#include "NaviMesh.h"
#include "NaviCell.h"
#include "ComponentSystem.h"
#include "MoveBroadcaster.h"
#include "SectorInfoHelper.h"

using namespace ServerCore;

NodeStatus MoveNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
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

NodeStatus RangeCheckNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
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

NodeStatus AttackNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
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

NodeStatus ChaseNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    const auto cur_pos = owner_comp_sys->GetComp<PositionComponent>()->pos;
    const auto pOwnerEntity = bt_root_timer->GetOwnerEntity();
    if (!awaker)return NodeStatus::FAILURE;
    
   // std::cout << cur_pos.x << ' ' << cur_pos.y << ' ' << cur_pos.z << '\n';
    
    const auto dest_pos = awaker->GetComp<PositionComponent>()->pos;
    
    const auto dx = dest_pos.x - cur_pos.x;
    const auto dy = dest_pos.y - cur_pos.y;
    const auto dz = dest_pos.z - cur_pos.z;
    
    auto dir = DirectX::SimpleMath::Vector3{ dx,dy,dz };
    dir.Normalize();
    
    const auto pNavi = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0);
    const auto dest = pNavi->FindRayIntersectingCell({ dest_pos.x,dest_pos.y,dest_pos.z });
    const auto start = pNavi->FindRayIntersectingCell({ cur_pos.x,cur_pos.y,cur_pos.z });

    if (start && dest)
    {
        const auto path = AStar::GetAStarPath(pNavi, start->GetCellIndex(pNavi), dest->GetCellIndex(pNavi), {dest_pos.x,dest_pos.y,dest_pos.z});
       
        if (path.size() >= 2)
        {
            const float dt_ = bt_root_timer->GetBTTimerDT();
            dir.x = path[path.size() - 1].x - cur_pos.x;
            dir.y = path[path.size() - 1].y - cur_pos.y;
            dir.z = path[path.size() - 1].z - cur_pos.z;
            dir.Normalize();

            const auto dx2 = cur_pos.x + dir.x * 5.2f * dt_;
            const auto dy2 = cur_pos.y + dir.y * 5.2f * dt_;
            const auto dz2 = cur_pos.z + dir.z * 5.2f * dt_;
            
            const auto pppp = pNavi->FindRayIntersectingCell({ dx2, dy2, dz2 });
            if (pppp)
            {
                pOwnerEntity->GetComp<PositionComponent>()->pos = Vec3{ dx2,pppp->CalculateY(dx2,dz2),dz2};
            }
            else
            pOwnerEntity->GetComp<PositionComponent>()->pos = Vec3{ dx2,std::abs(dy2),dz2 };

            ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };
            
            ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
            //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
            
        }
        else
        {
            const float dt_ = bt_root_timer->GetBTTimerDT();
            const auto dx2 = cur_pos.x + dir.x * 3.2f * dt_;
            const auto dy2 = cur_pos.y + dir.y * 3.2f * dt_;
            const auto dz2 = cur_pos.z + dir.z * 3.2f * dt_;

            pOwnerEntity->GetComp<PositionComponent>()->pos = Vec3{ dx2,dy2,dz2 };

            ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

            //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
            ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
        }

        return NodeStatus::RUNNING;
    }
    else
    {
        std::cout << "??" << std::endl;
    }

   //if (10 * 10 <= dx * dx + dy * dy + dz * dz) 
   //{
   //    awaker->GetSession()->SendAsync(Create_s2c_MONSTER_AGGRO_END((Nagox::Enum::GROUP_TYPE)pOwnerEntity->GetObjectType(), pOwnerEntity->GetObjectTypeInfo()));
   //    return NodeStatus::FAILURE;
   //}
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
    ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));

   // if (SECTOR_STATE::USER_EMPTY & sector_state) {
   //     return NodeStatus::FAILURE;
   // }

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

NodeStatus PatrolNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    owner_comp_sys->GetComp<PositionComponent>()->body_angle += 1000.f * bt_root_timer->GetBTTimerDT();
    const auto pOwnerEntity = owner_comp_sys->GetOwnerEntity();
    ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
    ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));

    //if (SECTOR_STATE::USER_EMPTY & sector_state)
    //    return NodeStatus::FAILURE;

    return NodeStatus::SUCCESS;
}
