#pragma once

struct Vec3
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

static inline Nagox::Struct::Vec3 ToFlatVec3(const Vec3& v)noexcept {
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
	Vec3 pos;
	Vec3 vel;
	Vec3 accel;
	float body_angle;
	uint64 time_stamp;
public:
	void foo(float x,float y)noexcept{}
private:

};

