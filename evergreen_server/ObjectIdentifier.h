#pragma once
#include "ContentsComponent.h"

class ObjectIdentifier
	: public ContentsComponent
{
public:
	CONSTRUCTOR_CONTENTS_COMPONENT(ObjectIdentifier)
public:
	virtual S_ptr<SendBuffer> CreateNotifyDetailPacket()const noexcept;
public:
	void BroadcastNotifyEquipmentChange()const noexcept;
private:
};

