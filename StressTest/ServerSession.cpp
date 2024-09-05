#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"

ServerSession::ServerSession()
	:ServerCore::PacketSession{ s2c_DummyPacketHandler::GetPacketHandlerList(),true }
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	std::cout << "Im Dummy !" << std::endl;
	SendAsync(Create_c2s_LOGIN("Hello"));
}

void ServerSession::OnSend(c_int32 len)noexcept
{
}

void ServerSession::OnDisconnected(const ID_Ptr<ServerCore::Sector> curSectorInfo_)noexcept
{
	IncRef();
	std::cout << "DisConnect !" << std::endl;
}
