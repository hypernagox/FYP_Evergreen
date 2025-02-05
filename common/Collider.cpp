#include "pch.h"
#include "Collider.h"

namespace Common
{
	bool Sphere::IsIntersect(const OBBBox* const other_obb) const noexcept { return Sphere::IsIntersect(other_obb->GetOBB()); }

	bool Sphere::IsIntersect(const Sphere* const other_sphere) const noexcept { return Sphere::IsIntersect(other_sphere->GetSphere()); }

	bool Sphere::IsIntersect(const AABBBox* const other_aabb) const noexcept { return Sphere::IsIntersect(other_aabb->GetAABB()); }

	bool AABBBox::IsIntersect(const OBBBox* const other_obb) const noexcept { return AABBBox::IsIntersect(other_obb->GetOBB()); }

	bool AABBBox::IsIntersect(const Sphere* const other_sphere) const noexcept { return AABBBox::IsIntersect(other_sphere->GetSphere()); }

	bool AABBBox::IsIntersect(const AABBBox* const other_aabb) const noexcept { return AABBBox::IsIntersect(other_aabb->GetAABB()); }

	bool OBBBox::IsIntersect(const OBBBox* const other_obb) const noexcept { return OBBBox::IsIntersect(other_obb->GetOBB()); }

	bool OBBBox::IsIntersect(const Sphere* const other_sphere) const noexcept { return OBBBox::IsIntersect(other_sphere->GetSphere()); }

	bool OBBBox::IsIntersect(const AABBBox* const other_aabb) const noexcept { return OBBBox::IsIntersect(other_aabb->GetAABB()); }
	
	bool Ray::IsIntersect(const OBBBox* const other_obb) const noexcept { return Ray::IsIntersect(other_obb->GetOBB()); }

	bool Ray::IsIntersect(const Sphere* const other_sphere) const noexcept { return Ray::IsIntersect(other_sphere->GetSphere()); }

	bool Ray::IsIntersect(const AABBBox* const other_aabb) const noexcept { return Ray::IsIntersect(other_aabb->GetAABB()); }
}