#pragma once
#include "pch.h"

class NetAddress
{
public:
	NetAddress()noexcept = default;
	NetAddress(SOCKADDR_IN sockAddr)noexcept;
	NetAddress(std::wstring_view ip, c_uint16 port)noexcept;

	SOCKADDR_IN& GetSockAddr()noexcept { return m_sockAddr; }
	const SOCKADDR_IN& GetSockAddr()const noexcept { return m_sockAddr; }
	std::wstring	GetIpAddress()const noexcept;
	uint16			GetPort()const noexcept { return ::ntohs(m_sockAddr.sin_port); }

public:
	static IN_ADDR	Ip2Address(const WCHAR* const ip)noexcept;

private:
	SOCKADDR_IN		m_sockAddr;
};
