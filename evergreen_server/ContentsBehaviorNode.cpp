#include "pch.h"
#include "ContentsBehaviorNode.h"
#include "TickTimer.h"
#include "Session.h"
#include "Navigator.h"
#include "NaviCell.h"
#include "ComponentSystem.h"
#include "MoveBroadcaster.h"
#include "NavigationMesh.h"
#include "NaviAgent_Common.h"
#include "PathFinder_Common.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "HP.h"

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

       // const auto session_list = pOwnerEntity->GetCurSector()->GetSessionCopyListIncRef();
        const auto& session_list = pOwnerEntity->GetCurCluster()->GetEntities(0);
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
          // s->DecRef();
        }
        if (target)
        {
            awaker_ = target->SharedFromThis();
            // TODO: 스트레스 테스트 아직 패킷없음
            //if(m_range * m_range > min_dist && prev_awaker != awaker_.get())
            //    awaker_->GetSession()->SendAsync(Create_s2c_MONSTER_AGGRO_START((Nagox::Enum::GROUP_TYPE)pOwnerEntity->GetObjectType(), pOwnerEntity->GetObjectTypeInfo()));
        }
    }
    

    
    if (!awaker_->IsValid())return NodeStatus::FAILURE;
    
    const auto dest_pos = awaker_->GetComp<PositionComponent>()->pos;

    const auto dx = dest_pos.x - cur_pos.x;
    const auto dy = dest_pos.y - cur_pos.y;
    const auto dz = dest_pos.z - cur_pos.z;

    const auto ag = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx);
  
    if (m_range * m_range <= dx * dx + dy * dy + dz * dz) 
    {
        NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->resetMoveTarget(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx);
        float temp[3]{};
        NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->requestMoveVelocity(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx, temp);
        
        ag->active = false;
        return NodeStatus::FAILURE;
    }
    ag->active = true;
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
        // TODO: 스트레스 테스트 아직 패킷없음
        awaker->GetSession()->SendAsync(Create_s2c_MONSTER_ATTACK(1));
        awaker->GetComp<HP>()->PostDoDmg(1,owner_comp_sys->GetOwnerEntity()->SharedFromThis());

        m_accTime = 1.5f;
    }

    return NodeStatus::RUNNING;
}

NodeStatus ChaseNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    const auto pOwnerEntity = bt_root_timer->GetOwnerEntity();
    if (!awaker)return NodeStatus::FAILURE;
     
    const auto dest_pos = awaker->GetComp<PositionComponent>()->pos;
    const auto cur_pos = pOwnerEntity->GetComp<PositionComponent>()->pos;
 
   // if (15 * 15 <= (dest_pos - cur_pos).LengthSquared())
   //     return NodeStatus::FAILURE;
    if (2 * 2 >= (dest_pos - cur_pos).LengthSquared())
    {
        const auto ag = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getEditableAgent(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx);
        ag->active = false;
        //std::cout << "추격 성공" << std::endl;
        NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->resetMoveTarget(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx);
        float temp[3]{};
        NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->requestMoveVelocity(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx, temp);
        return NodeStatus::SUCCESS;
    }


    const auto path = pOwnerEntity->GetComp<PathFinder>()->GetPath(cur_pos, dest_pos);
    {
   // ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    
        const auto ag = NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->getAgent(owner_comp_sys->GetComp<NaviAgent>()->m_my_idx);
       
        Vector3 ppp{ ag->npos[0],ag->npos[1],-ag->npos[2] };
        pOwnerEntity->GetComp<PositionComponent>()->pos = ppp;
        pOwnerEntity->GetComp<ServerCore::ClusterInfoHelper>()->BroadcastCluster(ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
        //ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
    }
    return NodeStatus::RUNNING;

    if(path.empty()) return NodeStatus::FAILURE;

    auto dir = path.size() >= 2 ? path[1] : path[0];
    dir -= cur_pos;

    dir.Normalize();
    
    const float dt_ = bt_root_timer->GetBTTimerDT();

    const auto dx2 = cur_pos.x + dir.x * 10.2f * dt_;
    const auto dy2 = cur_pos.y + dir.y * 10.2f * dt_;
    const auto dz2 = cur_pos.z + dir.z * 10.2f * dt_;

    pOwnerEntity->GetComp<NaviAgent>()->SetCellPos(cur_pos,Vector3{ dx2,dy2,dz2 });
    
   // ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    pOwnerEntity->GetComp<ServerCore::ClusterInfoHelper>()->BroadcastCluster(ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
   // ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
    

    if (2 * 2 >= (dest_pos - cur_pos).LengthSquared())
    {
        //std::cout << "추격 성공" << std::endl;
        return NodeStatus::SUCCESS;
    }

    
    return NodeStatus::RUNNING;
}

NodeStatus PatrolNode::Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    owner_comp_sys->GetComp<PositionComponent>()->body_angle += 1000.f * bt_root_timer->GetBTTimerDT();
    const auto pOwnerEntity = owner_comp_sys->GetOwnerEntity();
    //ServerCore::Vector<ServerCore::Sector*> sectors{ pOwnerEntity->GetCurSector() };

    //pOwnerEntity->MoveBroadcastEnqueue(0, 0, std::move(sectors));
    pOwnerEntity->GetComp<ServerCore::ClusterInfoHelper>()->BroadcastCluster(ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));
   // ServerCore::SectorInfoHelper::BroadcastWithID(bt_root_timer->GetCurObjInSight(), ServerCore::MoveBroadcaster::CreateMovePacket(pOwnerEntity));

    //if (SECTOR_STATE::USER_EMPTY & sector_state)
    //    return NodeStatus::FAILURE;

    return NodeStatus::SUCCESS;
}
