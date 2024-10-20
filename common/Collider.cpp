#include "pch.h"
#include "Collider.h"

namespace Common
{
	bool Collider::IsIntersect(const DirectX::BoundingBox& other) const noexcept
	{
		return m_box.Intersects(other);
	}
}