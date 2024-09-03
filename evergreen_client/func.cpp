#include "pch.h"
#include "func.h"


Vector3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept {
	return Vector3{ v->x(),v->y(),v->z() };
}

const Nagox::Struct::Vec3 ToFlatVec3(const Vector3& v)noexcept {
	return Nagox::Struct::Vec3{ v.x,v.y,v.z };
}
