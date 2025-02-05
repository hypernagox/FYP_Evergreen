#pragma once

namespace Common
{
	class Sphere;
	class OBBBox;
	class AABBBox;
	class Ray;

	class Collider
	{
	public:
		Collider()noexcept = default;
		~Collider()noexcept = default;
		Collider(const float pos[3])noexcept :m_pos{ pos } {}
		Collider(const DirectX::SimpleMath::Vector3* const pos)noexcept :Collider{ &pos->x } {}
	public:
		virtual bool IsCollision(const Collider* const other_col)const noexcept = 0;
	public:
		virtual bool IsIntersect(const OBBBox* const other_obb)const noexcept = 0;
		virtual bool IsIntersect(const Sphere* const other_sphere)const noexcept = 0;
		virtual bool IsIntersect(const AABBBox* const other_aabb)const noexcept = 0;
		virtual bool IsIntersect(const Ray& other_ray)const noexcept = 0;
	public:
		virtual bool IsIntersect(const DirectX::BoundingOrientedBox& othe)const noexcept = 0;
		virtual bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept = 0;
		virtual bool IsIntersect(const DirectX::BoundingBox& other)const noexcept = 0;
	public:
		const auto& GetPos()const noexcept { return *reinterpret_cast<const DirectX::SimpleMath::Vector3* const>(m_pos); }
		const auto GetPosWithOffset()const noexcept { return GetPos() + m_offSet; }
	public:
		DirectX::SimpleMath::Vector3 m_offSet = { 0,0,0 };
	private:
		const float* m_pos = nullptr;	// 각자 프로젝트의 포지션에 대한 포인터 (Vec3같은 플롯 3개짜리면 OK, 플롯3개짜리가 아니라면 UB임 ㅠ)
	};

	class Ray
		:public Collider
	{
	public:
		using Collider::Collider;
		Ray(const float pos[3])noexcept :Collider{ pos } {}
		Ray(const DirectX::SimpleMath::Vector3* const pos)noexcept :Collider{ pos } {}
	public:
		bool IsCollision(const Collider* const other_col)const noexcept override final { return other_col->IsIntersect(*this); }
	public:
		virtual bool IsIntersect(const OBBBox* const other_obb)const noexcept override final;
		virtual bool IsIntersect(const Sphere* const other_sphere)const noexcept override final;
		virtual bool IsIntersect(const AABBBox* const other_aabb)const noexcept override final;
		// TODO: 광선교차가 필요하다면 만든다
		bool IsIntersect(const Ray& other_ray)const noexcept override final { return false; }
	public:
		bool IsIntersect(const DirectX::BoundingOrientedBox& other)const noexcept override final { return other.Intersects(GetPosWithOffset(), m_dir, m_dist); }
		bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept override final { return other.Intersects(GetPosWithOffset(), m_dir, m_dist); }
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept override final { return other.Intersects(GetPosWithOffset(), m_dir, m_dist); }
	public:
		DirectX::SimpleMath::Vector3 m_dir;
		mutable float m_dist;
	};

	class Sphere
		:public Collider
	{
	public:
		using Collider::Collider;
		Sphere(const DirectX::SimpleMath::Vector3* const pos, const float radius)noexcept
			:Collider{ pos }, m_radius{ radius } {
		}
		Sphere(const float pos[3], const float radius)noexcept
			:Collider{ pos }, m_radius{ radius } {
		}
	public:
		bool IsCollision(const Collider* const other_col)const noexcept override final { return other_col->IsIntersect(this); }
	public:
		virtual bool IsIntersect(const OBBBox* const other_obb)const noexcept override final;
		virtual bool IsIntersect(const Sphere* const other_sphere)const noexcept override final;
		virtual bool IsIntersect(const AABBBox* const other_aabb)const noexcept override final;
		bool IsIntersect(const Ray& other_ray)const noexcept override final { return other_ray.IsIntersect(GetSphere()); }
	public:
		bool IsIntersect(const DirectX::BoundingOrientedBox& other)const noexcept override final { return GetSphere().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept override final { return GetSphere().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept override final { return GetSphere().Intersects(other); }
	public:
		DirectX::BoundingSphere GetSphere()const noexcept { return DirectX::BoundingSphere{ GetPosWithOffset(),m_radius }; }
	public:
		float m_radius;
	};

	class AABBBox
		:public Collider
	{
	public:
		using Collider::Collider;
		AABBBox(const DirectX::SimpleMath::Vector3* const pos, const DirectX::SimpleMath::Vector3& ex)noexcept
			:Collider{ pos }, m_extent{ ex } {
		}
		AABBBox(const float pos[3], const DirectX::SimpleMath::Vector3& ex)noexcept
			:Collider{ pos }, m_extent{ ex } {
		}
	public:
		bool IsCollision(const Collider* const other_col)const noexcept override final { return other_col->IsIntersect(this); }
	public:
		virtual bool IsIntersect(const OBBBox* const other_obb)const noexcept override final;
		virtual bool IsIntersect(const Sphere* const other_sphere)const noexcept override final;
		virtual bool IsIntersect(const AABBBox* const other_aabb)const noexcept override final;
		bool IsIntersect(const Ray& other_ray)const noexcept override final { return other_ray.IsIntersect(GetAABB()); }
	public:
		bool IsIntersect(const DirectX::BoundingOrientedBox& other)const noexcept override final { return GetAABB().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept override final { return GetAABB().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept override final { return GetAABB().Intersects(other); }
	public:
		DirectX::BoundingBox GetAABB()const noexcept { return DirectX::BoundingBox{ GetPosWithOffset(),m_extent }; }
	public:
		DirectX::SimpleMath::Vector3 m_extent;
	};

	class OBBBox
		:public Collider
	{
	public:
		using Collider::Collider;
		// TODO: 쿼터니언 또는 회전에대한 참조나 포인터 필요함
		OBBBox(const DirectX::SimpleMath::Vector3* const pos, const DirectX::SimpleMath::Vector3& ex)noexcept
			:Collider{ pos }, m_extent{ ex } {
		}
		OBBBox(const float pos[3], const DirectX::SimpleMath::Vector3& ex)noexcept
			:Collider{ pos }, m_extent{ ex } {
		}
	public:
		bool IsCollision(const Collider* const other_col)const noexcept override final { return other_col->IsIntersect(this); }
	public:
		virtual bool IsIntersect(const OBBBox* const other_obb)const noexcept override final;
		virtual bool IsIntersect(const Sphere* const other_sphere)const noexcept override final;
		virtual bool IsIntersect(const AABBBox* const other_aabb)const noexcept override final;
		bool IsIntersect(const Ray& other_ray)const noexcept override final { return other_ray.IsIntersect(GetOBB()); }
	public:
		bool IsIntersect(const DirectX::BoundingOrientedBox& other)const noexcept override final { return GetOBB().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept override final { return GetOBB().Intersects(other); }
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept override final { return GetOBB().Intersects(other); }
	public:
		DirectX::BoundingOrientedBox GetOBB()const noexcept { return DirectX::BoundingOrientedBox{ GetPosWithOffset(),m_extent ,m_rot }; }
	public:
		DirectX::SimpleMath::Vector3 m_extent;	// AABBBox랑 코드중복같지만, final 키워드 때문에 그냥 씀 (final 붙이면 컴파일러 최적화 여지가 있음)
		DirectX::SimpleMath::Quaternion m_rot = DirectX::SimpleMath::Quaternion::Identity; // TODO: 포지션컴포넌트의 회전포인터
	};

	class MultipleAABB
		:public Collider
	{	
		// 한 오브젝트가 AABB를 여러 개 달고있는 경우 (좀 복잡한 오브젝트?)
	public:
		using Collider::Collider;
		MultipleAABB(const DirectX::SimpleMath::Vector3* const mid_pos)noexcept
			:Collider{ mid_pos } {
		}
		MultipleAABB(const float mid_pos[3])noexcept
			:Collider{ mid_pos } {
		}
	public:
		void AddAABBBox(const DirectX::SimpleMath::Vector3& offSet, const DirectX::SimpleMath::Vector3& extent) {
			m_offSetAndExtents.emplace_back(std::make_pair(offSet, extent));
		}
	public:
		bool IsCollision(const Collider* const other)const noexcept override final {
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (other->IsIntersect(DirectX::BoundingBox{ GetPosWithOffset() + off,ex }))return true;
			}
			return false;
		}
	public:
		bool IsIntersect(const OBBBox* const other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (other->IsIntersect(DirectX::BoundingBox{ mid_pos + off,ex }))return true;
			}
			return false;
		}
		bool IsIntersect(const Sphere* const other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (other->IsIntersect(DirectX::BoundingBox{ mid_pos + off,ex }))return true;
			}
			return false;
		}
		bool IsIntersect(const AABBBox* const other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (other->IsIntersect(DirectX::BoundingBox{ mid_pos + off,ex }))return true;
			}
			return false;
		}
		bool IsIntersect(const Ray& other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (other.IsIntersect(DirectX::BoundingBox{ mid_pos + off,ex }))return true;
			}
			return false;
		}
	public:
		bool IsIntersect(const DirectX::BoundingOrientedBox& other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (DirectX::BoundingBox{ mid_pos + off,ex }.Intersects(other))return true;
			}
			return false;
		}
		bool IsIntersect(const DirectX::BoundingSphere& other)const noexcept override final {
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (DirectX::BoundingBox{ mid_pos + off,ex }.Intersects(other))return true;
			}
			return false;
		}
		bool IsIntersect(const DirectX::BoundingBox& other)const noexcept override final{ 
			const auto mid_pos = GetPosWithOffset();
			for (const auto& [off, ex] : m_offSetAndExtents) {
				if (DirectX::BoundingBox{ mid_pos + off,ex }.Intersects(other))return true;
			}
			return false;
		}
	private:
		std::vector<std::pair<DirectX::SimpleMath::Vector3, DirectX::SimpleMath::Vector3>> m_offSetAndExtents;
	};

	// 사실 원뿔검사니까 콘이 더 맞을지도..
	class Fan
	{
	public:
		Fan(const DirectX::SimpleMath::Vector3& pos,
			const DirectX::SimpleMath::Vector3& dir,
			const float deg,
			const float radius)noexcept
			:m_pos{ pos }, m_dir{ dir }, m_degree{ deg }, m_radius{ radius }
		{
			m_dir.Normalize();
		}
	public:
		bool IsIntersect(const Collider* const col) const noexcept { return IsIntersect(col->GetPosWithOffset()); }
		bool IsIntersect(const Vector3& point) const noexcept
		{
			const Vector3 OP = point - m_pos + m_offSet;
			const float len = OP.Length();
			return (len < FLT_EPSILON) ||
				   (len <= m_radius &&
				   m_dir.Dot(OP) >= len * DirectX::XMScalarCosEst(DirectX::XMConvertToRadians(m_degree * 0.5f)));
		}
	public:
		DirectX::SimpleMath::Vector3 m_pos;
		DirectX::SimpleMath::Vector3 m_offSet = {};
		DirectX::SimpleMath::Vector3 m_dir;
		float m_degree;
		float m_radius;
	};
}

