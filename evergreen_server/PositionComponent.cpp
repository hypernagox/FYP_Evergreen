#include "pch.h"
#include "PositionComponent.h"

std::pair<float, float> PositionComponent::GetXZGlobal(const ContentsEntity* const entity) noexcept
{
    return entity->GetComp<PositionComponent>()->GetXZ();
}

std::pair<float, float> PositionComponent::GetXZWithOffsetGlobal(const ContentsEntity* const entity) noexcept
{
    return entity->GetComp<PositionComponent>()->GetXZWithOffset();
}
