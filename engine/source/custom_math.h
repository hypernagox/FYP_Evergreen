#pragma once

#include "pch.h"

namespace udsdx
{
	constexpr float PI = 3.1415926535897932384626433832795f;
	constexpr float PI2 = PI * 2.0f;
	constexpr float PIDIV2 = PI / 2.0f;
	constexpr float PIDIV4 = PI / 4.0f;
	constexpr float DEG2RAD = PI / 180.0f;
	constexpr float RAD2DEG = 180.0f / PI;

	// Linear interpolation
	template <typename T>
	inline T Lerp(const T a, const T b, const T t)
	{
		return a + (b - a) * t;
	}

	// Cosine interpolation approximation
	template <typename T>
	inline T Cerp(const T a, const T b, const T t)
	{
		T ct = std::clamp<T>(t, 0, 1);
		T tt = ct * 2 > 1 ? 1 - ct : ct;
		tt = tt * tt;
		tt = tt * tt * 8;
		return Lerp<T>(a, b, ct * 2 > 1 ? 1 - tt : tt);
	}
}