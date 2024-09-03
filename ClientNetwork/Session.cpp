#include "ClientNetworkPch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "NetEvents.h"
#include "PacketSession.h"

namespace NetHelper
{
	Session::Session()
		: m_pRecvEvent{ std::make_unique<RecvEvent>() }
		, m_pConnectEvent{ std::make_unique<ConnectEvent>() }
		, m_pDisconnectEvent{ std::make_unique<DisconnectEvent>() }
		, m_pSendEvent{ std::make_unique<SendEvent>() }
		, m_pRecvBuffer{ std::make_unique<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ SocketUtils::CreateSocket() }
	{
	}

	Session::~Session()
	{
	}

	void Session::Send(S_ptr<SendBuffer> pSendBuff_)noexcept
	{
		m_sendQueue.emplace_back(std::move(pSendBuff_));
		if (false == std::exchange(m_bIsSendRegistered, true))
			RegisterSend();
	}

	void Session::Disconnect(std::wstring cause)
	{
		std::wcout << cause << std::endl;

		if (!m_bConnected)
			return;
		OnDisconnected();
		m_bConnected = false;
		RegisterDisconnect();
	}

	bool Session::SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept
	{
		return SocketUtils::SetTcpNoDelay(m_sessionSocket, bTrueIsOff_FalseIsOn);
	}

	bool Session::RegisterConnect()
	{
		if (IsConnected())
			return false;
		if (false == SocketUtils::SetReuseAddress(m_sessionSocket, true))
			return false;
		if (false == SocketUtils::BindAnyAddress(m_sessionSocket, 0))
			return false;

		m_pConnectEvent->Init();
		m_pConnectEvent->SetSession(m_pCacheSharedFromThis);

		const SOCKADDR_IN& sockAddr = m_serverAddr.GetSockAddr(); // 내가 붙어야 할 서버쪽 주소임

		//DWORD numOfBytes;

		if (false == SocketUtils::ConnectEx(m_sessionSocket, reinterpret_cast<const SOCKADDR* const>(&sockAddr), sizeof(sockAddr), NULL, NULL, NULL, m_pConnectEvent.get()))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				m_pConnectEvent->ReleaseSession();
				return false;
			}
		}

		return true;
	}

	void Session::ProcessConnect(c_int32 numofBytes_)
	{
		m_bConnected = true;

		m_pConnectEvent->ReleaseSession();

		OnConnected();

		RegisterRecv();
	}

	bool Session::RegisterDisconnect()
	{
		m_pDisconnectEvent->Init();
		m_pDisconnectEvent->SetSession(m_pCacheSharedFromThis);

		if (false == SocketUtils::DisconnectEx(m_sessionSocket, m_pDisconnectEvent.get(), TF_REUSE_SOCKET, 0))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				m_pDisconnectEvent->ReleaseSession();
				return false;
			}
		}

		return true;
	}

	void Session::ProcessDisconnect(c_int32 numofBytes_)
	{
		m_pDisconnectEvent->ReleaseSession();
	}

	void Session::RegisterRecv()noexcept
	{
		if (false == IsConnected())
			return;

		m_pRecvEvent->Init();
		m_pRecvEvent->SetSession(m_pCacheSharedFromThis);

		WSABUF wsaBuf{ static_cast<const ULONG>(m_pRecvBuffer->FreeSize()),reinterpret_cast<char* const>(m_pRecvBuffer->WritePos()) };
		//DWORD numOfBytes;
		DWORD flags = 0;
		if (SOCKET_ERROR == ::WSARecv(m_sessionSocket, &wsaBuf, 1, NULL, &flags, m_pRecvEvent.get(), RecvCompletionRoutine))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				m_pRecvEvent->ReleaseSession();
			}
		}
	}

	void Session::ProcessRecv(c_int32 numofBytes_)
	{
		m_pRecvEvent->ReleaseSession();

		if (0 == numofBytes_)
		{
			Disconnect(L"Recv 0");
			return;
		}

		if (false == m_pRecvBuffer->OnWrite(numofBytes_)) [[unlikely]]
		{
			Disconnect(L"OnWrite Overflow");
			return;
		}

		const int32 dataSize = m_pRecvBuffer->DataSize();

		const int32 processLen = static_cast<PacketSession* const>(this)->PacketSession::OnRecv(m_pRecvBuffer->ReadPos(), dataSize);

		if (processLen < 0 || dataSize < processLen || false == m_pRecvBuffer->OnRead(processLen))
		{
			Disconnect(L"OnRecv Overflow");
			return;
		}

		m_pRecvBuffer->Clear();

		RegisterRecv();
	}

	void Session::RegisterSend()noexcept
	{
		if (false == IsConnected())
			return;

		m_pSendEvent->Init();
		m_pSendEvent->SetSession(m_pCacheSharedFromThis);

		m_sendQueue.swap(m_pSendEvent->sendBuffer);

		int32 writeSize = 0;

		const DWORD bufferCount = static_cast<const DWORD>(m_pSendEvent->sendBuffer.size());
		std::vector<WSABUF> wsaBufs; wsaBufs.reserve(bufferCount);
		for (const auto& sb : m_pSendEvent->sendBuffer)
		{
			wsaBufs.emplace_back(static_cast<const ULONG>(sb->WriteSize()), reinterpret_cast<char* const>(sb->Buffer()));
			writeSize += sb->WriteSize();
		}
		//DWORD numOfBytes = 0;
		if (SOCKET_ERROR == ::WSASend(m_sessionSocket, wsaBufs.data(), bufferCount, NULL, 0, m_pSendEvent.get(), SendCompletionRoutine))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				m_pSendEvent->ReleaseSession();

				m_pSendEvent->sendBuffer.clear();
				m_bIsSendRegistered = false;
			}
		}
	}

	void Session::ProcessSend(c_int32 numofBytes_)
	{
		m_pSendEvent->ReleaseSession();
		m_pSendEvent->sendBuffer.clear();

		if (0 == numofBytes_)
		{
			Disconnect(L"Send 0");
			return;
		}

		OnSend(numofBytes_);

		if (m_sendQueue.empty())
		{
			m_bIsSendRegistered = false;
		}
		else
		{
			RegisterSend();
		}
	}

	void Session::HandleError(c_int32 errorCode)
	{
		switch (errorCode)
		{
		case WSAECONNRESET:
		case WSAECONNABORTED:
			Disconnect(L"HandleError");
			break;
		default:
			// TODO 로그찍기
			std::wcout << std::format(L"Handle Error: {}", errorCode) << std::endl;
			break;
		}
	}

	void RecvCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		static_cast<RecvEvent* const>(lpOverlapped)->GetSession()->ProcessRecv(dwNumberOfBytesTransfered);
	}

	void SendCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		static_cast<SendEvent* const>(lpOverlapped)->GetSession()->ProcessSend(dwNumberOfBytesTransfered);
	}
}