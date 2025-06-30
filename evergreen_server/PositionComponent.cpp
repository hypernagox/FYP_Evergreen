#include "pch.h"
#include "PositionComponent.h"
#include "ClusterInfoHelper.h"
#include "Cluster.h"
#include "NaviAgent_Common.h"
#include "Navigator.h"

std::pair<float, float> PositionComponent::GetXZGlobal(const ContentsEntity* const entity) noexcept
{
    return entity->GetComp<PositionComponent>()->GetXZ();
}

std::pair<float, float> PositionComponent::GetXZWithOffsetGlobal(const ContentsEntity* const entity) noexcept
{
    return entity->GetComp<PositionComponent>()->GetXZWithOffset();
}

void PositionComponent::AdjustMovement(const float dt, const Vector3 desired_dir) noexcept
{
    //TODO: 매직넘버
    Vector3 steer = desired_dir;

    const auto owner = GetOwnerEntityRaw();
   // const auto clusters = owner->GetComp<NagiocpX::ClusterInfoHelper>()->GetAdjClusters(owner);
    const auto clusters = XVector<NagiocpX::Cluster*>{ owner->GetCurCluster() };
    constexpr const float NEAR_MON_DIST_SQ = 5.f / 2.f * 5.f / 2.f;
    constexpr const float COMBINED_RADIUS = 2.5f + 3.5f;
    constexpr const float COMBINED_RADIUS_SQ = COMBINED_RADIUS * COMBINED_RADIUS;
    constexpr const float HARD_PUSH_DIST_SQ = 1.5f;
    constexpr const float MOVE_SPEED = 2.5f;

    for (const auto cluster : clusters)
    {
        for (const auto& npcs : cluster->GetEntitesExceptSession())
        {
            for (const auto npc : npcs.GetItemListRef())
            {
                if (npc == owner)
                    continue;

                const auto other = npc->GetComp<PositionComponent>();
                const Vector3 rel_pos = other->pos - pos;
                const float dist_sq = rel_pos.LengthSquared();

                if (dist_sq >= NEAR_MON_DIST_SQ)
                    continue;

                const Vector3 avoid_dir = -CommonMath::Normalized(rel_pos);

                if (dist_sq < HARD_PUSH_DIST_SQ)
                {
                    steer += avoid_dir * 5.0f;
                    continue;
                }
                else if (dist_sq < COMBINED_RADIUS_SQ)
                {
                    const float push_weight = std::clamp(
                        (COMBINED_RADIUS_SQ - dist_sq) / COMBINED_RADIUS_SQ, 0.f, 1.f);
                    const Vector3 soft_steer = steer + avoid_dir * push_weight;
                    steer = CommonMath::Normalized(steer * 0.6f + soft_steer * 0.4f);
                }
            }
        }
    }

    const Vector3 final_dir = CommonMath::Normalized(steer * 0.9f + desired_dir * 0.1f);
    Vector3 delta_move = final_dir * MOVE_SPEED * dt;

    if (delta_move.LengthSquared() < 1e-6f)
        return;

    pos += delta_move;

    Vector3 corrected_pos = pos;
    owner->GetComp<NaviAgent>()->SetCellPos(dt, pos, corrected_pos);
}