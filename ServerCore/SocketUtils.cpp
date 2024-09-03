#include "ServerCorePch.h"
#include "SocketUtils.h"

/*----------------
	SocketUtils
-----------------*/

namespace ServerCore
{
	LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
	LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
	LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

	const HANDLE SocketUtils::Init()noexcept
	{
		WSADATA wsaData;
		NAGOX_ASSERT(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0);

		/* 런타임에 주소 얻어오는 API */
		SOCKET dummySocket = CreateSocket();
		NAGOX_ASSERT(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
		NAGOX_ASSERT(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
		NAGOX_ASSERT(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
		Close(dummySocket);

		return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NULL);
	}

	void SocketUtils::Clear()noexcept
	{
		::WSACleanup();
	}

	bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* const fn)noexcept
	{
		DWORD bytes = 0;
		return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
	}

	SOCKET SocketUtils::CreateSocket()noexcept
	{
		return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)noexcept
	{
		LINGER option;
		option.l_onoff = onoff;
		option.l_linger = linger;
		return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
	}

	bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)noexcept
	{
		return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, static_cast<int>(flag));
	}

	bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)noexcept
	{
		return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
	}

	bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)noexcept
	{
		return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
	}

	bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag) noexcept
	{
		return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, static_cast<int>(flag));
	}

	// ListenSocket의 특성을 ClientSocket에 그대로 적용
	bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)noexcept
	{
		return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
	}

	bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)noexcept
	{
		return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
	}

	bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)noexcept
	{
		SOCKADDR_IN myAddress;
		myAddress.sin_family = AF_INET;
		myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
		myAddress.sin_port = ::htons(port);

		return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
	}

	bool SocketUtils::Listen(SOCKET socket, int32 backlog)noexcept
	{
		return SOCKET_ERROR != ::listen(socket, backlog);
	}

	bool SocketUtils::Close(SOCKET& socket)noexcept
	{
		if (INVALID_SOCKET == socket)
			return false;

		::closesocket(socket);
		socket = INVALID_SOCKET;
		return true;
	}
}