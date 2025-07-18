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
	template <typename TV, typename TT>
	inline TV Lerp(const TV a, const TV b, const TT t)
	{
		return a + (b - a) * t;
	}

	// Cosine interpolation approximation
	template <typename TV, typename TT>
	inline TV Cerp(const TV a, const TV b, const TT t)
	{
		TT ct = std::clamp<TT>(t, 0, 1);
		TT tt = ct * 2 > 1 ? 1 - ct : ct;
		tt = tt * tt;
		tt = tt * tt * 8;
		return Lerp<TV, TT>(a, b, ct * 2 > 1 ? 1 - tt : tt);
	}

	// Wrap Around Interpolation [-180, 180]
	inline float LerpAngle(const float a, const float b, const float t)
	{
		float delta = std::fmodf((b - a + 540.0f), 360.0f) - 180.0f;
		return std::fmodf((a + delta * t + 180.0f), 360.0f) - 180.0f;
	}

	// Wrap Around Interpolation [-¥ð, ¥ð]
	inline float LerpAngleRadian(const float a, const float b, const float t)
	{
		float delta = std::fmodf((b - a + PI2 + PI), PI2) - PI;
		return std::fmodf((a + delta * t + PI), PI2) - PI;
	}

	static constexpr float SmoothStep(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}
}