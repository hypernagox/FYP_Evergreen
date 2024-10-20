#pragma once
#include "Collider.h"
#include "ContentsComponent.h"

class PositionComponent;

class Collider
	:public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(Collider)

public:
	const DirectX::BoundingBox& GetBox(const Vector3& offset = {})noexcept;
	bool IsCollision(const DirectX::BoundingBox& other)noexcept;
	void SetBox(PositionComponent* p, Vector3 ex);
private:
	Common::Collider m_collider;
	PositionComponent* m_posComp;
};

