#pragma once
#include "pch.h"

class PositionComponent
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(PositionComponent)
public:
	const auto& GetPktPos()const noexcept { return ToFlatVec(pos); }
	const auto& GetPktVel()const noexcept { return ToFlatVec(vel); }
	const auto& GetPktAccel()const noexcept { return ToFlatVec(accel); }
public:
	void SetPos(const auto& v)noexcept { pos = ToDxVec(v); }
	void SetVel(const auto& v)noexcept { vel = ToDxVec(v); }
	void SetAccel(const auto& v)noexcept { accel = ToDxVec(v); }
public:
	void SetMovementInfo(const Nagox::Protocol::c2s_MOVE& pkt)noexcept {
		SetPos(pkt.pos());
		SetVel(pkt.vel());
		SetAccel(pkt.accel());
		body_angle = pkt.body_angle();
		time_stamp = pkt.time_stamp();
	}
public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	float body_angle;
	uint64 time_stamp = 0;
};

