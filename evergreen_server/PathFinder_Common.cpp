#include "pch.h"
#include "PathFinder_Common.h"
#include "NaviAgent_Common.h"
#include "PositionComponent.h"

std::span<Vector3> PathFinder::GetPath(const Vector3& start, const Vector3& dest) const noexcept
{
    return m_pathFinder.GetPath(start, dest);
}
