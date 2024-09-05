#pragma once
#include "PacketSession.h"

class ServerSession
	:public ServerCore::PacketSession
{
public:
	ServerSession();
	~ServerSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override;
	virtual void OnDisconnected(const ID_Ptr<ServerCore::Sector> curSectorInfo_)noexcept override;
public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	uint64 m_timeStamp = ::GetTickCount64();
	uint64 m_id = 0;
private:

};

