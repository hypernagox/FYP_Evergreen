#include "pch.h"
#include "ContentsFunc.h"
#include "PositionComponent.h"

XVector<std::pair<S_ptr<ContentsEntity>, class PositionComponent*>> CreateArrWithPosComp(const XVector<const ContentsEntity*>& vec_) noexcept
{
    auto b = vec_.data();
    const auto e = b + vec_.size();
    XVector<std::pair<S_ptr<ContentsEntity>, class PositionComponent*>> temp;
    temp.reserve(e - b);
    while (e != b) {
        const auto entity = (*b++);
        const auto pos_comp = entity->GetComp<PositionComponent>();
        temp.emplace_back(std::make_pair(entity, pos_comp));
    }

    return temp;
}
