#include "pch.h"
#include "RangeMonState.h"
#include "PositionComponent.h"
#include "PathFinder_Common.h"
#include "NaviAgent_Common.h"
#include "MoveBroadcaster.h"
#include "Projectile.h"
#include "TimerRoutine.h"

const State::EntityState RangeMonIdle::Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
	if (!awaker)
		return RANGE_MON_STATE::IDLE;

	if (IsInDistEntity(awaker.get(), comp_sys->GetOwnerEntity(), 40))
		return RANGE_MON_STATE::CHASE;

    return RANGE_MON_STATE::IDLE;
}

const State::EntityState RangeMonChase::Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    if (!awaker)
        return RANGE_MON_STATE::IDLE;

    const auto dest_pos = awaker->GetComp<PositionComponent>()->pos;
    const auto cur_pos = comp_sys->GetComp<PositionComponent>()->pos;

    const auto path = comp_sys->GetComp<PathFinder>()->GetPath(cur_pos, dest_pos);

    if (CommonMath::IsInDistanceDX(dest_pos, cur_pos, 20))return RANGE_MON_STATE::ATTACK;
    if (path.empty()) return RANGE_MON_STATE::IDLE;

    auto dir = path.size() >= 2 ? path[1] : path[0];
    dir -= cur_pos;

    dir.Normalize();


    const auto dx2 = cur_pos.x + dir.x * 5.2f * dt;
    const auto dy2 = cur_pos.y + dir.y * 5.2f * dt;
    const auto dz2 = cur_pos.z + dir.z * 5.2f * dt;

    comp_sys->GetComp<NaviAgent>()->SetCellPos(cur_pos, Vector3{ dx2,dy2,dz2 });

    comp_sys->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / CommonMath::C_PI;

   

    ServerCore::TickTimer::BroadcastObjInSight(ServerCore::TickTimer::GetTempVecForInsightObj(), ServerCore::MoveBroadcaster::CreateMovePacket(comp_sys->GetOwnerEntity()));

    return RANGE_MON_STATE::CHASE;
}

const State::EntityState RangeMonAttack::Update(const float dt, const ComponentSystemNPC* const comp_sys, const S_ptr<ServerCore::ContentsEntity>& awaker) noexcept
{
    if (!awaker)
        return RANGE_MON_STATE::IDLE;
       
    if (!IsInDistEntity(awaker.get(), comp_sys->GetOwnerEntity(), 25))
        return RANGE_MON_STATE::CHASE;
    if (0.f < m_acc) {
        m_acc -= dt;
        return RANGE_MON_STATE::ATTACK;
    }
    m_acc = 2.f;
    const auto proj = ServerCore::TimerHandler::CreateTimerWithoutHandle<MonProjectile>(100);
    proj.timer->m_pos = (comp_sys->GetComp<PositionComponent>()->pos);

    auto dir = awaker->GetComp<PositionComponent>()->pos - proj.timer->m_pos;
    dir.Normalize();
   
    proj.timer->m_speed = dir * 10.f;
   
    proj.timer->SelectObjList(ServerCore::TickTimer::GetTempVecForInsightObj());
    proj.timer->m_owner = comp_sys->GetOwnerEntity()->SharedFromThis();

    return RANGE_MON_STATE::ATTACK;

}
