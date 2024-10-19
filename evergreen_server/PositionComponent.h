#pragma once

static inline Nagox::Struct::Vec3 ToFlatVec3(const Vector3& v)noexcept {
	return Nagox::Struct::Vec3{ v.x,v.y,v.z };
}

//static inline Vec3 ToOriginVec3(const Nagox::Struct::Vec3* const v)noexcept {
//	return Vec3{ v->x(),v->y(),v->z() };
//}

class PositionComponent
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(PositionComponent)
public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	float body_angle;
	uint64 time_stamp;
public:
	
private:

};

