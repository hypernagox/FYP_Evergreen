#pragma once
#include "PacketSession.h"

class ClientSession
	:public NagiocpX::PacketSession
{
public:
	ClientSession()noexcept;
	~ClientSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override {}
	virtual void OnDisconnected(const NagiocpX::Cluster* const curCluster_)noexcept override;
public:
	//template<typename T = ClientSession>
	//NagiocpX::S_ptr<T> SharedFromThis()const noexcept { return NagiocpX::S_ptr<T>{this}; }
private:

};

