#pragma once

class PositionComponent
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(PositionComponent)
public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	float body_angle;
	uint64 time_stamp = 0;
public:
	
private:

};

