#include "ServerCorePch.h"
#include "NetAddress.h"

/*--------------
	NetAddress
---------------*/

namespace ServerCore
{
	NetAddress::NetAddress(SOCKADDR_IN sockAddr)noexcept : _sockAddr(sockAddr)
	{
	}

	NetAddress::NetAddress(std::wstring_view ip, c_uint16 port)noexcept
	{
		::memset(&_sockAddr, NULL, sizeof(_sockAddr));
		_sockAddr.sin_family = AF_INET;
		_sockAddr.sin_addr = Ip2Address(ip.data());
		_sockAddr.sin_port = ::htons(port);
	}

	const std::wstring_view NetAddress::GetIpAddress()const noexcept
	{
		constinit thread_local WCHAR buffer[64] = {};
		::InetNtopW(AF_INET, &_sockAddr.sin_addr, (WCHAR*)::memset(buffer, 0, sizeof(buffer)), len32(buffer));
		return buffer;
	}

	IN_ADDR NetAddress::Ip2Address(const WCHAR* const ip)noexcept
	{
		IN_ADDR address;
		::InetPtonW(AF_INET, ip, &address);
		return address;
	}
}