#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN sockAddr)noexcept : m_sockAddr(sockAddr)
{
}

NetAddress::NetAddress(std::wstring_view ip, c_uint16 port)noexcept
{
	::memset(&m_sockAddr, NULL, sizeof(m_sockAddr));
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_addr = Ip2Address(ip.data());
	m_sockAddr.sin_port = ::htons(port);
}

std::wstring NetAddress::GetIpAddress()const noexcept
{
	WCHAR buffer[64];
	::InetNtopW(AF_INET, &m_sockAddr.sin_addr, buffer, len32(buffer));
	return std::wstring{ buffer };
}

IN_ADDR NetAddress::Ip2Address(const WCHAR* const ip)noexcept
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}