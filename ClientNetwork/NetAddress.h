#pragma once

/*--------------
	NetAddress
---------------*/

namespace NetHelper
{
	class NetAddress
	{
	public:
		NetAddress()noexcept = default;
		NetAddress(SOCKADDR_IN sockAddr)noexcept;
		NetAddress(std::wstring_view ip, c_uint16 port)noexcept;

		const SOCKADDR_IN& GetSockAddr()const noexcept { return _sockAddr; }
		std::wstring GetIpAddress()const noexcept;
		uint16 GetPort() const noexcept { return ::ntohs(_sockAddr.sin_port); }

	public:
		static IN_ADDR	Ip2Address(const WCHAR* const ip)noexcept;

	private:
		SOCKADDR_IN	_sockAddr;
	};
}