#include "pch.h"
#include "ServerSession.h"

ServerSession::ServerSession()
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	std::cout << "On Connected !\n";
}

void ServerSession::OnSend(c_int32 len)
{
}

void ServerSession::OnDisconnected()
{
	std::cout << "On Disconnected !\n";
}