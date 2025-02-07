#include "NagiocpXPch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"
#include "RecvBuffer.h"
#include "PacketSession.h"

namespace NagiocpX
{
	Listener::Listener()
	{
	}

	Listener::~Listener()
	{
		CancelIoEx(reinterpret_cast<HANDLE>(m_socket), NULL);
		shutdown(m_socket, SD_BOTH);
		SocketUtils::Close(m_socket);
	}

	bool Listener::StartAccept(ServerService* const pServerService_)
	{
		m_pServerService = pServerService_;

		if (nullptr == m_pServerService)
			return false;
		
		m_socket = SocketUtils::CreateSocket();

		if (INVALID_SOCKET == m_socket)
			return false;
		if (false == m_pServerService->GetIocpCore().RegisterIOCP(this))
			return false;
		if (false == SocketUtils::SetReuseAddress(m_socket, true))
			return false;
		if (false == SocketUtils::SetLinger(m_socket, 0, 0))
			return false;
		if (false == SocketUtils::Bind(m_socket, m_pServerService->GetNetAddress()))
			return false;
		if (false == SocketUtils::Listen(m_socket))
			return false;

		// Accept하는 스레드는 반드시 하나로 보장해야함
		constexpr const int32 acceptCount = 1;

		for (int i = 0; i < acceptCount; ++i)
		{
			m_acceptEvent = MakeSharedSTD<AcceptEvent>();
			m_acceptEvent->SetIocpObject(SharedFromThis<IocpObject>());
			RegisterAccept(m_acceptEvent.get());
		}

		return true;
	}

	void Listener::CloseAccept()
	{
		// TODO: 연결 안받기 , 다시받기
		m_bCanAccept.store(false);
		std::atomic_thread_fence(std::memory_order_seq_cst);
	}

	void Listener::FinishServer() noexcept
	{
		//for (auto& accepts : m_vecAcceptEvent)
		//{
		//	const auto session = accepts->ReleaseSession();
		//	//::closesocket(session->GetSocket());
		//	aligned_xdelete_sized<ContentsEntity>(session->GetOwnerEntity(), sizeof(ContentsEntity), alignof(ContentsEntity));
		//	accepts->ReleaseIocpObject();
		//}
		if (const auto session = m_acceptEvent->ReleaseSession())
		{
			session->GetOwnerEntity()->DecRef();
			m_acceptEvent->ReleaseIocpObject();
		}
	}

	void Listener::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept
	{
		NAGOX_ASSERT(iocpEvent_->GetEventType<EVENT_TYPE>() == EVENT_TYPE::ACCEPT);
		AcceptEvent* const acceptEvent = static_cast<AcceptEvent*>(iocpEvent_);
		ProcessAccept(acceptEvent->PassSession(), acceptEvent);
	}

	void Listener::RegisterAccept(AcceptEvent* const acceptEvent)noexcept
	{
		RETRY_ACCEPT:
		if (false == CanAccept())
			return;

		const auto ov_ptr = acceptEvent->Init();
		const S_ptr<Session>& session = acceptEvent->RegisterSession(m_pServerService->PopSession());

		_Post_ _Notnull_ ov_ptr;

		if (!session)
		{
			m_bCanAccept.store(false);
			if (false == m_pServerService->IsSessionPoolEmpty() && false == m_bCanAccept.exchange(true))
			{
				if (Mgr(ThreadMgr)->GetStopFlagRef())
					return;
				else
					goto RETRY_ACCEPT;
			}
			else
			{
				PrintLogEndl("Server Is Full or Fail to CreateSession\n");
				return;
			}
		}

		//DWORD bytesReceived;

		if (false == SocketUtils::AcceptEx(m_socket, session->GetSocket(), session->m_pRecvBuffer->WritePos(), 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, ov_ptr))
		{
			const int32 errCode = ::WSAGetLastError();
			if (errCode != WSA_IO_PENDING)
			{
				xdelete<ContentsEntity>(acceptEvent->ReleaseSession()->GetOwnerEntity());
				PrintLogEndl(std::format("Accept Error: {}", errCode));
				Sleep(5000);
				if (Mgr(ThreadMgr)->GetStopFlagRef())
					return;
				else
					goto RETRY_ACCEPT;
			}
		}
	}

	void Listener::ProcessAccept(S_ptr<class PacketSession> pSession, AcceptEvent* const acceptEvent)noexcept
	{
		const auto session_ptr = pSession.get();

		if (!session_ptr)
		{
			return;
		}

		const SOCKET sessionSocket = session_ptr->GetSocket();

		if (false == SocketUtils::SetUpdateAcceptSocket(sessionSocket, m_socket))
		{
			xdelete<ContentsEntity>(session_ptr->GetOwnerEntity());
			RegisterAccept(acceptEvent);
			return;
		}

		if (false == SocketUtils::SetTcpNoDelay(sessionSocket, true))
		{
			xdelete<ContentsEntity>(session_ptr->GetOwnerEntity());
			RegisterAccept(acceptEvent);
			return;
		}

		SOCKADDR_IN sockAddress;
		int32 sizeOfSockAddr = sizeof(sockAddress);
		if (SOCKET_ERROR == ::getpeername(sessionSocket, reinterpret_cast<SOCKADDR* const>(&sockAddress), &sizeOfSockAddr))
		{
			xdelete<ContentsEntity>(session_ptr->GetOwnerEntity());
			RegisterAccept(acceptEvent);
			return;
		}

		session_ptr->SetNetAddress(NetAddress{ sockAddress });
		session_ptr->ProcessConnect(std::move(pSession));

		RegisterAccept(acceptEvent);
	}
}