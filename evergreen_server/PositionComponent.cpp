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
    // TODO: 매직넘버 조절
    Vector3 steer = desired_dir;

    const auto owner = GetOwnerEntityRaw();
    const auto clusters = owner->GetComp<NagiocpX::ClusterInfoHelper>()->GetAdjClusters(owner);

    constexpr const float NEAR_MON_DIST = 5.0f * 5.0f;

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

                if (dist_sq >= NEAR_MON_DIST)
                    continue;

                constexpr const float combined_radius = 1.5f + 1.5f;
                if (dist_sq < combined_radius * combined_radius)
                {
                    const float push_weight = std::clamp((combined_radius * combined_radius - dist_sq) / (combined_radius * combined_radius), 0.f, 1.f);
                    const Vector3 avoid_dir = -CommonMath::Normalized(rel_pos);
                    steer += avoid_dir * push_weight * 3.25f;
                }
            }
        }
    }

    const Vector3 final_dir = CommonMath::Normalized(steer);
    constexpr float move_speed = 2.5f;
    const Vector3 delta_move = final_dir * move_speed * dt;

   // delta_move.y = 0.f;

    pos += delta_move;

    Vector3 corrected_pos = pos;
    owner->GetComp<NaviAgent>()->SetCellPos(dt, pos, corrected_pos);
    // pos = corrected_pos;
}
