#pragma once
#include "Collider.h"
#include "ContentsComponent.h"
#include "PositionComponent.h"

class PositionComponent;

class Collider
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(Collider)
public:
	template<typename T = Common::Collider>
	const auto GetCollider()const noexcept { return reinterpret_cast<T*>(((char*)(&m_posComp)) + sizeof(m_posComp)); }
	template<typename T>
	bool IsCollision(const T& other)const noexcept { return GetCollider<T>()->IsIntersect(other); }
	const auto GetPosComp()const noexcept { return m_posComp; }
protected:
	PositionComponent* m_posComp{ nullptr };
	// �ݵ�� ��ӹ��� ���� ù ����� �ݶ��̴� ����ü �� ��ü������
};

class SphereCollider
	:public Collider
{
	using Collider::Collider;
public:
	Common::Sphere* SetSphere(PositionComponent* posComp, float rad)noexcept {
		NAGOX_ASSERT(nullptr == m_posComp);
		m_posComp = posComp;
		return std::construct_at<Common::Sphere>(&m_collider, &posComp->pos, rad);
	}
public:
	Common::Sphere m_collider;
};

class AABBCollider
	:public Collider
{
	using Collider::Collider;
public:
	Common::AABBBox* SetAABB(PositionComponent* posComp, const Vector3& extent)noexcept {
		NAGOX_ASSERT(nullptr == m_posComp);
		m_posComp = posComp;
		return std::construct_at<Common::AABBBox>(&m_collider, &posComp->pos, extent);
	}
public:
	Common::AABBBox m_collider;
};

class OBBCollider
	:public Collider
{
	using Collider::Collider;
public:
	//TODO: ȸ�����ʹϾ� �����ͳ� ���۷���
	Common::OBBBox* SetOBB(PositionComponent* posComp, const Vector3& extent)noexcept {
		NAGOX_ASSERT(nullptr == m_posComp);
		m_posComp = posComp;
		return std::construct_at<Common::OBBBox>(&m_collider, &posComp->pos, extent);
	}
public:
	Common::OBBBox m_collider;
};


