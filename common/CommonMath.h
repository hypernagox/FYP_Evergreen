#pragma once
#include "pch.h"

namespace CommonMath
{
	inline Vector3 Normalized(const Vector3& v)noexcept
	{
		Vector3 temp = v; temp.Normalize();
		return temp;
	}

	inline Vector3 InverseZ(const Vector3& v)noexcept
	{
		return Vector3{ v.x,v.y,-v.z };
	}

	inline void InverseZ(Vector3& v)noexcept
	{
		v.z = -v.z;
	}

	static inline float GetYawFromQuaternion(const DirectX::SimpleMath::Quaternion& q)noexcept
	{
		return atan2(2.0f * (q.y * q.w + q.x * q.z), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
	}
}
