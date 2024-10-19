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
}
