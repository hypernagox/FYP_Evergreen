#include "ServerCorePch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "PacketSession.h"
#include "Queueabler.h"
#include "MoveBroadcaster.h"

namespace ServerCore
{
	extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
	thread_local XVector<S_ptr<SendBuffer>> clear_vec = {};

	Session::Session()noexcept
		: m_pOwnerEntity{ xnew<ContentsEntity>(this) }
		, m_pDisconnectEvent{ MakeUniqueSized<DisconnectEvent>() }
		, m_pSendEvent{ xnew<SendEvent>(SocketUtils::CreateSocket()) }
		, m_pRecvEvent{ xnew<RecvEvent>() }
		, m_pRecvBuffer{ aligned_xnew<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ m_pSendEvent->m_sendSocket }
	{
		GetRefCountExternal(this) = 2;
	}

	Session::Session(const bool bNeedConnect) noexcept
		: m_pOwnerEntity{ xnew<ContentsEntity>(this) }
		, m_pConnectEvent{ MakeUniqueSized<ConnectEvent>() }
		, m_pDisconnectEvent{ MakeUniqueSized<DisconnectEvent>() }
		, m_pSendEvent{ xnew<SendEvent>(SocketUtils::CreateSocket()) }
		, m_pRecvEvent{ xnew<RecvEvent>() }
		, m_pRecvBuffer{ aligned_xnew<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ m_pSendEvent->m_sendSocket }
	{
		GetRefCountExternal(this) = 2;
	}

	Session::~Session()
	{
		xdelete<SendEvent>(m_pSendEvent);
		xdelete<RecvEvent>(m_pRecvEvent);
		aligned_xdelete<RecvBuffer>(m_pRecvBuffer, alignof(RecvBuffer));
		SocketUtils::Close(m_sessionSocket);
		NAGOX_ASSERT_LOG(0 != m_serviceIdx, "Session Double Free !");
		m_serviceIdx = 0;
	}

	bool Session::Connect()
	{
		return RegisterConnect();
	}

	bool Session::Disconnect(const std::wstring_view cause, S_ptr<PacketSession> move_session)noexcept
	{
		//LOG_MSG(std::move(cause));
		if (false == m_bConnected.exchange(false))
			return false;
		m_bConnectedNonAtomicForRecv = m_bConnectedNonAtomic = false;
		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true);
		RegisterDisconnect(std::move(move_session));
		PrintLogEndl(std::format(L"Err: {}, Cd: {}", cause, ::WSAGetLastError()));
		return true;
	}

	bool Session::SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept
	{
		return SocketUtils::SetTcpNoDelay(m_sessionSocket, bTrueIsOff_FalseIsOn);
	}

	void Session::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept
	{
		(this->*g_sessionLookupTable[static_cast<const uint8_t>(iocpEvent_->GetEventType<EVENT_TYPE>())])(iocpEvent_->PassIocpObject(), numOfBytes);
	}

	void Session::ClearSessionForReuse() noexcept
	{
		GetRefCountExternal(this) = 2;
		m_pRecvBuffer->ResetBufferCursor();
		m_bIsSendRegistered = false;
		m_bTurnOfZeroRecv = true;
		m_pOwnerEntity = xnew<ContentsEntity>(this); // 오너 엔티티가 딜리트되었음을 보장하고 함
		m_sendQueue.clear_single();
		m_pSendEvent->m_sendBuffer.clear();
	}

	bool Session::RegisterConnect()
	{
		if (IsConnected())
			return false;
		if (SERVICE_TYPE::CLIENT != GetService()->GetServiceType())
			return false;
		if (false == SocketUtils::SetReuseAddress(m_sessionSocket, true))
			return false;
		if (false == SocketUtils::BindAnyAddress(m_sessionSocket, 0))
			return false;

		const SOCKET connect_socket = m_sessionSocket;
		ConnectEvent* const connect_event = m_pConnectEvent.get();

		connect_event->Init();
		const auto ov_ptr = connect_event->GetOverlappedAddr();
		connect_event->SetIocpObject(SharedFromThis<IocpObject>());

		_Post_ _Notnull_ ov_ptr;

		const SOCKADDR_IN& sockAddr = GetService()->GetNetAddress().GetSockAddr();

		if (false == SocketUtils::ConnectEx(connect_socket, reinterpret_cast<const SOCKADDR* const>(&sockAddr), sizeof(sockAddr), NULL, NULL, NULL, ov_ptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode, connect_event->PassIocpObject());
				return false;
			}
		}

		return true;
	}

	void Session::ProcessConnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		if (GetService()->AddSession(pThisSessionPtr))
		{
			m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = true;
			const RecvBuffer* const recv_buff = m_pRecvBuffer;
			m_bConnected.store(true);

			OnConnected();
			
			RegisterRecv(std::move(pThisSessionPtr), recv_buff);
		}
		else
		{
			xdelete<ContentsEntity>(GetOwnerEntity());
		}
	}

	bool Session::RegisterDisconnect(S_ptr<PacketSession>&& move_session)noexcept
	{
		const SOCKET disconnect_socket = m_sessionSocket;
		DisconnectEvent* const disconnect_event = m_pDisconnectEvent.get();

		disconnect_event->Init();
		const auto ov_ptr = disconnect_event->GetOverlappedAddr();
		disconnect_event->SetIocpObject(std::move(move_session));

		::CancelIoEx(reinterpret_cast<HANDLE>(disconnect_socket), NULL);

		if (false == SocketUtils::DisconnectEx(disconnect_socket, ov_ptr, TF_REUSE_SOCKET, 0))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				S_ptr<Session> temp_session{ disconnect_event->PassIocpObject() };

				::CancelIoEx(reinterpret_cast<HANDLE>(disconnect_socket), NULL);
				SocketUtils::Close(m_sessionSocket);
				m_sessionSocket = m_pSendEvent->m_sendSocket = SocketUtils::CreateSocket();
				::CreateIoCompletionPort((HANDLE)m_sessionSocket, IocpCore::GetIocpHandleGlobal(), 0, 0);

				ContentsEntity* const pOwner = GetOwnerEntity();
				pOwner->TryOnDestroy();
				GetService()->ReleaseSession(this);
				pOwner->DecRef();

				HandleError(errorCode, std::move(temp_session));

				return false;
			}
		}
		return true;
	}

	void Session::ProcessDisconnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		ContentsEntity* const pOwner = GetOwnerEntity();
		pOwner->TryOnDestroy();
		GetService()->ReleaseSession(this);
		pOwner->DecRef();
	}

	void Session::RegisterRecv(S_ptr<PacketSession>&& pThisSessionPtr, const RecvBuffer* const pRecvBuffPtr)noexcept
	{
		WSABUF wsaBuf = (m_bTurnOfZeroRecv)
		? WSABUF{ 0,nullptr }
		: WSABUF{ static_cast<const ULONG>(pRecvBuffPtr->FreeSize()),reinterpret_cast<char* const>(pRecvBuffPtr->WritePos()) };
		
		if (false == m_bConnectedNonAtomicForRecv)
		{
			Disconnect(L"", std::move(pThisSessionPtr));
			return;
		}

		const SOCKET recv_socket = m_sessionSocket;
		RecvEvent* const recv_event = m_pRecvEvent;
	
		recv_event->Init();
		const auto ov_ptr = recv_event->GetOverlappedAddr();
		recv_event->SetIocpObject(std::move(pThisSessionPtr));
		DWORD flags = 0;
		
		if (SOCKET_ERROR == ::WSARecv(recv_socket, &wsaBuf, 1, NULL, &flags, ov_ptr, nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode, recv_event->PassIocpObject());
			}
		}
	}

	void Session::ProcessRecv(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		RecvBuffer* const recv_buff = m_pRecvBuffer;

		if (m_bTurnOfZeroRecv = !m_bTurnOfZeroRecv)
		{
			if (0 == numofBytes_)
			{
				Disconnect(L"Recv 0", std::move(pThisSessionPtr));
				return;
			}

			if (false == recv_buff->OnWrite(numofBytes_)) [[unlikely]]
			{
				Disconnect(L"OnWrite Overflow", std::move(pThisSessionPtr));
				return;
			}

			const int32_t dataSize = recv_buff->DataSize(); 
			
			const RecvStatus recvStatus = static_cast<PacketSession* const>(this)->PacketSession::OnRecv(recv_buff->ReadPos(), dataSize, pThisSessionPtr);

			if (false == recvStatus.bIsOK || recvStatus.processLen < 0 || dataSize < recvStatus.processLen || false == recv_buff->OnRead(recvStatus.processLen)) [[unlikely]]
			{
				Disconnect(L"Invalid Recv", std::move(pThisSessionPtr));
				return;
			}

			recv_buff->Clear();
		}

		RegisterRecv(std::move(pThisSessionPtr), recv_buff);
	}
	
	void Session::RegisterSend(S_ptr<PacketSession>&& pThisSessionPtr)noexcept
	{
		extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
		
		auto& wsabuf_storage = new_view_list_session.GetItemListRef();

		if (false == IsConnected())
		{
			Disconnect(L"", std::move(pThisSessionPtr));
			return;
		}
		
		SendEvent* const send_event = m_pSendEvent;
		const SOCKET send_socket = send_event->m_sendSocket;

		send_event->Init();
		const auto ov_ptr = send_event->GetOverlappedAddr();
		send_event->SetIocpObject(std::move(pThisSessionPtr));
		
		auto& sendBuffer = send_event->m_sendBuffer;
		m_sendQueue.try_flush_single(sendBuffer);

		const auto num = static_cast<const DWORD>(sendBuffer.size());

		if (SOCKET_ERROR == ::WSASend(send_socket, reinterpret_cast<WSABUF*>(wsabuf_storage.data()), num, NULL, 0, ov_ptr, nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				return RetrySendAsError(send_event);
			}
		}
	}

	void Session::ProcessSend(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		extern thread_local XVector<S_ptr<SendBuffer>> clear_vec;

		auto& sendBuffer = m_pSendEvent->m_sendBuffer;
		
		clear_vec.swap(sendBuffer);

		if (0 == numofBytes_)
		{
			Disconnect(L"Send 0", std::move(pThisSessionPtr));
			clear_vec.clear();
			return;
		}

		// TODO: Send후 해야할 일이 생겼을 때 다시 하자
		// OnSend(numofBytes_);

		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, false);

		// empty체크 후, 컨텍스트 스위칭... 이후 남이 send다하고 그다음 돌아와서 뒤늦게 플래그걸면 성공하는데 비어있음
		if (!m_sendQueue.empty_single() && false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
			RegisterSend(std::move(pThisSessionPtr));

		clear_vec.clear();
	}

	void Session::RetrySendAsError(SendEvent* const pSendEvent_)noexcept
	{
		S_ptr<Session> temp_session{ pSendEvent_->PassIocpObject() };
		auto& register_send_event = m_registerSendEvent;
		const auto ov_ptr = register_send_event.GetOverlappedAddr();
		const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
		PrintLogEndl(std::format(L"Send Error: {}", ::WSAGetLastError()));
		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, false);
		if (false == m_sendQueue.empty_single())
		{
			if (false == m_bIsSendRegistered &&
				false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
			{
				RegisterSend(std::move(temp_session));
				PrintLogEndl("Retry Send As Error");
			}
		}
	}

	void Session::RetrySend() const noexcept
	{
		auto& register_send_event = m_registerSendEvent;
		const auto ov_ptr = register_send_event.GetOverlappedAddr();
		const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, false);
		if (false == m_sendQueue.empty_single())
		{
			if (false == m_bIsSendRegistered &&
				false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
			{
				register_send_event.SetIocpObject(SharedFromThis<IocpObject>());
				::PostQueuedCompletionStatus(iocp_handle, 0, 0, ov_ptr);
				// PrintLogEndl("Retry Send!");
			}
		}
	}

	void Session::HandleError(c_int32 errorCode, S_ptr<PacketSession>&& move_session)noexcept
	{
		m_iLastErrorCode = errorCode;
		if (false == Disconnect(std::format(L"HErr: {}", errorCode), std::move(move_session)))
			PrintLogEndl(std::format(L"HErr: {}", errorCode));
	}
}
