#pragma once
#include "ServerComponent.h"

class PositionSetter
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(PositionSetter)
public:
	// 나중에 뭔가 순간적으로 강제로 위치이동 시킬 때 사용할 것
	virtual void Update()noexcept override{}
private:

};

