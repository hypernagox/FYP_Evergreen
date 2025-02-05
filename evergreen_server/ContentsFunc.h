#pragma once
#include "ServerCorePch.h"
using ServerCore::ContentsEntity;

static inline Vector3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept {
	return Vector3{ v->x(),v->y(),v->z() };
}

static inline Nagox::Struct::Vec3 ToFlatVec3(const Vector3& v)noexcept {
	return Nagox::Struct::Vec3{ v.x,v.y,v.z };
}

float GetDistPow(const ContentsEntity* const a, const ContentsEntity* const b)noexcept;

inline bool IsInDistEntity(const ContentsEntity* const a, const ContentsEntity* const b, const float dist)noexcept {
	return GetDistPow(a, b) <= (dist * dist);
}