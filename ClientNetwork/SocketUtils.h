#pragma once
#include "ClientNetworkPch.h"
#include "NetAddress.h"

/*----------------
	SocketUtils
-----------------*/

namespace NetHelper
{
	class SocketUtils
	{
	public:
		SocketUtils() = delete;
		~SocketUtils() = delete;

	public:
		static LPFN_CONNECTEX		ConnectEx;
		static LPFN_DISCONNECTEX	DisconnectEx;
		static LPFN_ACCEPTEX		AcceptEx;

	public:
		static void Init()noexcept;
		static void Clear()noexcept;

		static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* const fn)noexcept;
		static SOCKET CreateSocket()noexcept;

		static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger)noexcept;
		static bool SetReuseAddress(SOCKET socket, bool flag)noexcept;
		static bool SetRecvBufferSize(SOCKET socket, int32 size)noexcept;
		static bool SetSendBufferSize(SOCKET socket, int32 size)noexcept;
		static bool SetTcpNoDelay(SOCKET socket, bool flag)noexcept;
		static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)noexcept;

		static bool Bind(SOCKET socket, NetAddress netAddr)noexcept;
		static bool BindAnyAddress(SOCKET socket, uint16 port)noexcept;
		static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN)noexcept;
		static bool Close(SOCKET& socket)noexcept;
	};

	template<typename T>
	static inline bool SetSockOpt(const SOCKET socket, c_int32 level, c_int32 optName, T optVal)noexcept
	{
		static_assert(sizeof(int) <= sizeof(T));
		return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<const char* const>(&optVal), sizeof(T));
	}
}