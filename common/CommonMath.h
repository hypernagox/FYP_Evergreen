#pragma once
#include "pch.h"

namespace CommonMath
{
	static inline constexpr const float C_PI = 3.141592f; // PI는 너무 흔한이름이라 나중에 문제생길까봐

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

	static inline float GetYawFromQuaternion(const DirectX::SimpleMath::Quaternion& q) noexcept
	{
		using namespace DirectX;

		const XMVECTOR quat = XMLoadFloat4(&q);
		const XMVECTOR two = XMVectorReplicate(2.0f);
		const XMVECTOR x = XMVectorSplatX(quat);
		const XMVECTOR y = XMVectorSplatY(quat);
		const XMVECTOR z = XMVectorSplatZ(quat);
		const XMVECTOR w = XMVectorSplatW(quat);
		const XMVECTOR num = XMVectorMultiply(two, XMVectorAdd(XMVectorMultiply(y, w), XMVectorMultiply(x, z)));
		const XMVECTOR denom = XMVectorSubtract(XMVectorReplicate(1.0f), XMVectorMultiply(two, XMVectorAdd(XMVectorMultiply(y, y), XMVectorMultiply(z, z))));

		return XMVectorGetX(XMVectorATan2(num, denom));
	}

	inline float GetDistPowDX(const Vector3& a, const Vector3& b)noexcept {
		return Vector3::DistanceSquared(a, b);
	}

	inline float GetDistPowDX(const float a[3], const float b[3])noexcept {
		return GetDistPowDX(*(Vector3*)a, *(Vector3*)b);
	}
	
	inline bool IsInDistanceDX(const Vector3& a, const Vector3& b, const float th_hold)noexcept {
		return GetDistPowDX(a, b) <= (th_hold * th_hold);
	}

	inline bool IsInDistanceDX(const float a[3], const float b[3], const float th_hold)noexcept {
		return IsInDistanceDX(*(Vector3*)a, *(Vector3*)b, th_hold);
	}

	
}
