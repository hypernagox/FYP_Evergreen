#pragma once
#include "ServerComponent.h"

class PositionSetter
	:public ServerComponent
{
public:
	CONSTRUCTOR_SERVER_COMPONENT(PositionSetter)
public:
	// ���߿� ���� ���������� ������ ��ġ�̵� ��ų �� ����� ��
	virtual void Update()noexcept override{}
private:

};

