#pragma once
#include "NagiocpXPch.h"
using NagiocpX::ContentsEntity;

static inline const Vector3& ToDxVec(const float v[3])noexcept {
	return *reinterpret_cast<const Vector3* const>(v);
}
static inline const Nagox::Struct::Vec3& ToFlatVec(const float v[3])noexcept {
	return *reinterpret_cast<const Nagox::Struct::Vec3* const>(v);
}
static inline const Vector3& ToDxVec(const Nagox::Struct::Vec3* const v)noexcept { return ToDxVec((const float*)v); }

static inline const Nagox::Struct::Vec3& ToFlatVec(const Vector3& v)noexcept { return ToFlatVec((const float*)&v); }

float GetDistPow(const ContentsEntity* const a, const ContentsEntity* const b)noexcept;

inline bool IsInDistEntity(const ContentsEntity* const a, const ContentsEntity* const b, const float dist)noexcept {
	return GetDistPow(a, b) <= (dist * dist);
}