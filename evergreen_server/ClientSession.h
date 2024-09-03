#pragma once
#include "PacketSession.h"

class ClientSession
	:public ServerCore::PacketSession
{
public:
	ClientSession()noexcept;
	~ClientSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override {}
	virtual void OnDisconnected(const ID_Ptr<ServerCore::Sector> curSectorInfo_)noexcept override;
public:
	//template<typename T = ClientSession>
	//ServerCore::S_ptr<T> SharedFromThis()const noexcept { return ServerCore::S_ptr<T>{this}; }
private:

};

