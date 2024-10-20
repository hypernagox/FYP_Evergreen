#include "pch.h"
#include "Collider_Common.h"
#include "PositionComponent.h"

const DirectX::BoundingBox& Collider::GetBox(const Vector3& offset) noexcept
{
	auto& box = m_collider.m_box;
	box.Center = m_posComp->pos + offset;
	return box;
}

bool Collider::IsCollision(const DirectX::BoundingBox& other) noexcept
{
	return GetBox().Intersects(other);
}

void Collider::SetBox(PositionComponent* p, Vector3 ex)
{
	m_posComp = p;
	m_collider.m_box.Extents = ex;
}
