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
	thread_local Vector<WSABUF> wsaBufs(128);

	Session::Session(const PacketHandleFunc* const sessionPacketHandler_)noexcept
		: m_pOwnerEntity{ aligned_xnew<ContentsEntity>(this) }
		, m_pRecvEvent{ aligned_xnew<RecvEvent>() }
		, m_pDisconnectEvent{ MakeUniqueSized<DisconnectEvent>() }
		, m_pSendEvent{ aligned_xnew<SendEvent>() }
		, m_pRecvBuffer{ MakeUniqueSized<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ SocketUtils::CreateSocket() }
		, m_sessionPacketHandler{ sessionPacketHandler_ }
		, m_sessionSocketForRecv{ m_sessionSocket }
	{
		//IncRef();
		GetRefCountExternal(this) = 2;
		//GetRefCountExternal(this).store(2, std::memory_order_release);
	}

	Session::Session(const PacketHandleFunc* const sessionPacketHandler_, const bool bNeedConnect) noexcept
		: m_pOwnerEntity{ aligned_xnew<ContentsEntity>(this) }
		, m_pRecvEvent{ aligned_xnew<RecvEvent>() }
		, m_pConnectEvent{ MakeUniqueSized<ConnectEvent>() }
		, m_pDisconnectEvent{ MakeUniqueSized<DisconnectEvent>() }
		, m_pSendEvent{ aligned_xnew<SendEvent>() }
		, m_pRecvBuffer{ MakeUniqueSized<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ SocketUtils::CreateSocket() }
		, m_sessionPacketHandler{ sessionPacketHandler_ }
		, m_sessionSocketForRecv{ m_sessionSocket }
	{
		//IncRef();
		GetRefCountExternal(this) = 2;
	//	GetRefCountExternal(this).store(2, std::memory_order_release);
	}

	Session::~Session()
	{
		aligned_xdelete_sized<RecvEvent>(m_pRecvEvent, sizeof(RecvEvent), alignof(RecvEvent));
		aligned_xdelete_sized<SendEvent>(m_pSendEvent, sizeof(SendEvent), alignof(SendEvent));
		SocketUtils::Close(m_sessionSocket);
		NAGOX_ASSERT_LOG(0 != m_serviceIdx, "Session Double Free !");
		m_serviceIdx = 0;
	}

	bool Session::Connect()
	{
		return RegisterConnect();
	}

	bool Session::Disconnect(const std::wstring_view cause)noexcept
	{
		//LOG_MSG(std::move(cause));
		if (false == m_bConnected.exchange(false))
			return false;
		m_bConnectedNonAtomicForRecv = m_bConnectedNonAtomic = false;
		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true);
		RegisterDisconnect();
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

		const SOCKET connect_socket = m_sessionSocketForRecv;
		ConnectEvent* const connect_event = m_pConnectEvent.get();

		connect_event->Init();
		connect_event->SetIocpObject(SharedFromThis<IocpObject>());
		

		const SOCKADDR_IN& sockAddr = GetService()->GetNetAddress().GetSockAddr(); // 내가 붙어야 할 서버쪽 주소임

		//DWORD numOfBytes = 0;

		if (false == SocketUtils::ConnectEx(connect_socket, reinterpret_cast<const SOCKADDR* const>(&sockAddr), sizeof(sockAddr), NULL, NULL, NULL, connect_event->GetOverlappedAddr()))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				const S_ptr<Session> temp_session{ connect_event->PassIocpObject() };
				return false;
			}
		}

		return true;
	}

	void Session::ProcessConnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		// 세선 등록
		if (GetService()->AddSession(pThisSessionPtr))
		{
			m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = true;
			const RecvBuffer* const recv_buff = m_pRecvBuffer.get();

			m_bConnected.store(true);
			// 컨텐츠 코드에서 오버로딩 해야함
			// 입장시 해야할 일

			OnConnected();

			// 수신 등록(낚싯대 던짐)
			RegisterRecv(std::move(pThisSessionPtr), recv_buff);
		}
		else
		{
			aligned_xdelete_sized<ContentsEntity>(GetOwnerEntity(), sizeof(ContentsEntity), alignof(ContentsEntity));
		}
	}

	bool Session::RegisterDisconnect()noexcept
	{
		const SOCKET disconnect_socket = m_sessionSocket;
		DisconnectEvent* const disconnect_event = m_pDisconnectEvent.get();

		disconnect_event->Init();
		disconnect_event->SetIocpObject(SharedFromThis<IocpObject>());

		::shutdown(disconnect_socket, SD_BOTH);

		if (false == SocketUtils::DisconnectEx(disconnect_socket, disconnect_event->GetOverlappedAddr(), TF_REUSE_SOCKET, 0))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				const S_ptr<Session> temp_session{ disconnect_event->PassIocpObject() };

				m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = false;
				m_bConnected.store(false);

				//::CancelIoEx(reinterpret_cast<HANDLE>(disconnect_socket), NULL);
				//SocketUtils::Close(m_sessionSocket);

				ContentsEntity* const pOwner = GetOwnerEntity();
				pOwner->TryOnDestroy();
				GetService()->ReleaseSession(this);
				pOwner->GetComp<MoveBroadcaster>()->ReleaseViewList();
				pOwner->DecRef();

				return false;
			}
		}
		return true;
	}

	void Session::ProcessDisconnect(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		//SocketUtils::Close(m_sessionSocket);
		
		ContentsEntity* const pOwner = GetOwnerEntity();
		pOwner->TryOnDestroy();
		GetService()->ReleaseSession(this);
		pOwner->GetComp<MoveBroadcaster>()->ReleaseViewList();
		pOwner->DecRef();
	}

	void Session::RegisterRecv(S_ptr<PacketSession>&& pThisSessionPtr, const RecvBuffer* const pRecvBuffPtr)noexcept
	{
		WSABUF wsaBuf{ static_cast<const ULONG>(pRecvBuffPtr->FreeSize()),reinterpret_cast<char* const>(pRecvBuffPtr->WritePos()) };
		
		if (false == m_bConnectedNonAtomicForRecv)
		{
			Disconnect(L"");
			return;
		}

		const SOCKET recv_socket = m_sessionSocketForRecv;
		RecvEvent* const recv_event = m_pRecvEvent;
	
		recv_event->Init();
		recv_event->SetIocpObject(std::move(pThisSessionPtr));
		DWORD flags = 0;

		if (SOCKET_ERROR == ::WSARecv(recv_socket, &wsaBuf, 1, NULL, &flags, recv_event->GetOverlappedAddr(), nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				const S_ptr<Session> temp_session{ recv_event->PassIocpObject() };
				HandleError(errorCode);
			}
		}
	}

	void Session::ProcessRecv(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		if (0 == numofBytes_)
		{
			Disconnect(L"Recv 0");
			return;
		}
		
		RecvBuffer* const recv_buff = m_pRecvBuffer.get();

		if (false == recv_buff->OnWrite(numofBytes_)) [[unlikely]]
		{
			Disconnect(L"OnWrite Overflow");
			return;
		}

		const int32 dataSize = recv_buff->DataSize(); // 더 읽어야할 데이터 w - r
		// 컨텐츠 쪽에서 오버로딩 해야함

		const RecvStatus recvStatus = static_cast<PacketSession* const>(this)->PacketSession::OnRecv(recv_buff->ReadPos(), dataSize, pThisSessionPtr);

		if (false == recvStatus.bIsOK || recvStatus.processLen < 0 || dataSize < recvStatus.processLen || false == recv_buff->OnRead(recvStatus.processLen)) [[unlikely]]
		{
			Disconnect(L"OnRecv Overflow");
			return;
		}

		recv_buff->Clear(); // 커서 정리

		RegisterRecv(std::move(pThisSessionPtr), recv_buff);
	}
	
	void Session::RegisterSend(S_ptr<PacketSession>&& pThisSessionPtr)noexcept
	{
		extern thread_local Vector<WSABUF> wsaBufs;

		if (false == IsConnected())
		{
			// TODO: 이래도 메모리릭 없는지 확인 필요
			// 디스커넥트 시점을 철저히 Recv때에만 맞추기 위함
			 Disconnect(L"");
			return;
		}
		
		SendEvent* const send_event = m_pSendEvent;
		const SOCKET send_socket = m_sessionSocket;

		send_event->Init();
		send_event->SetIocpObject(std::move(pThisSessionPtr));

		auto& sendBuffer = send_event->m_sendBuffer;
		m_sendQueue.try_flush_single(sendBuffer);

		const auto num = static_cast<const DWORD>(sendBuffer.size());

		if (0 == num)
		{
			S_ptr<Session> temp_session{ send_event->PassIocpObject() };
			InterlockedExchange8((CHAR*)&m_bIsSendRegistered, false);
			std::this_thread::yield();
			if (false == m_sendQueue.empty_single())
			{
				if (false == m_bIsSendRegistered &&
					false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
				{
					const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
					auto& register_send_event = m_pSendEvent->m_registerSendEvent;
					register_send_event.SetIocpObject(std::move(temp_session));
					::PostQueuedCompletionStatus(iocp_handle, 0, 0, register_send_event.GetOverlappedAddr());
				}
			}
			return;
		}

		if (SOCKET_ERROR == ::WSASend(send_socket, wsaBufs.data(), num, NULL, 0, send_event->GetOverlappedAddr(), nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				const S_ptr<Session> temp_session{ send_event->PassIocpObject() };
				 HandleError(errorCode);
				// TODO: Send에서 뷰리스트 정리를 하지 않기 위함
			}
		}
	}

	void Session::ProcessSend(S_ptr<PacketSession> pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		auto& sendBuffer = m_pSendEvent->m_sendBuffer;
		const Vector<S_ptr<SendBuffer>> temp{ std::move(sendBuffer) };
		
		if (0 == numofBytes_)
		{
			// TODO: 이래도 메모리릭 없는지 확인 필요
			// 디스커넥트 시점을 철저히 Recv때에만 맞추기 위함
			Disconnect(L"Send 0");
			return;
		}

		sendBuffer.reserve(temp.size());

		// TODO: Send후 해야할 일이 생겼을 때 다시 하자
		// OnSend(numofBytes_);

		InterlockedExchange8((CHAR*)&m_bIsSendRegistered, false);

		if (!m_sendQueue.empty_single() && false == InterlockedExchange8((CHAR*)&m_bIsSendRegistered, true))
			RegisterSend(std::move(pThisSessionPtr));
	}

	void Session::HandleError(c_int32 errorCode)noexcept
	{
		Disconnect(L"HandleError");
		switch (errorCode)
		{
		case WSAECONNRESET:
		case WSAECONNABORTED:
			//Disconnect(L"HandleError");
			break;
		default:
			// TODO 로그찍기
			//LOG_MSG(std::format(L"Handle Error: {}", errorCode));
			break;
		}
	}
}
