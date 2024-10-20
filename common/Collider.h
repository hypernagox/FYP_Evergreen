#pragma once

namespace Common
{
	class Collider
	{
	public:
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept;
		DirectX::BoundingBox m_box;
	};
}

