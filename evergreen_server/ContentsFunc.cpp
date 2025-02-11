#include "pch.h"
#include "ContentsFunc.h"
#include "PositionComponent.h"

float GetDistPow(const ContentsEntity* const a, const ContentsEntity* const b) noexcept
{
    return CommonMath::GetDistPowDX(a->GetComp<PositionComponent>()->pos, b->GetComp<PositionComponent>()->pos);
}
