#pragma once
#include "pch.h"

class ServerSession
	:public NetHelper::PacketSession
{
public:
	ServerSession();
	~ServerSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)override;
	virtual void OnDisconnected()override;
private:
	// 자기자신 게임오브젝트 들고있을거임
	// 만약 부캐 있으면 배열임
};

