#include "ClientNetworkPch.h"
#include "NetworkMgr.h"
#include "SocketUtils.h"
#include "SendBufferMgr.h"

namespace NetHelper
{
    NetworkMgr::NetworkMgr()
    {
        SocketUtils::Init();
        m_connectEvent = ::WSACreateEvent();
        if (WSA_INVALID_EVENT == m_connectEvent)
        {
            NET_NAGOX_ASSERT_LOG(false, "Invalide Socket");
        }
    }

    NetworkMgr::~NetworkMgr()
    {
    }

    void NetworkMgr::DoNetworkIO(const DWORD timeout_millisecond) noexcept
    {
        const DWORD dwResult = WSAWaitForMultipleEvents(1, &m_connectEvent, FALSE, timeout_millisecond, TRUE);
        WSANETWORKEVENTS networkEvents;
        if (WSAEnumNetworkEvents(m_c2sSession->GetSocket(), m_connectEvent, &networkEvents) != SOCKET_ERROR)
        {
            if (networkEvents.lNetworkEvents & FD_CLOSE)
            {
                m_c2sSession->Disconnect(L"Bye");
                if (m_disconnectCallback)m_disconnectCallback();
            }
        }
    }

    void NetworkMgr::SetSessionID(const uint32_t sessionID_) const noexcept
    {
        NET_NAGOX_ASSERT_LOG(0 == m_c2sSession->GetSessionID() && 0 != sessionID_, "Session ID Must Init Once !");
        m_c2sSession->SetSessionID(sessionID_);
    }

    c_uint32 NetworkMgr::GetSessionID() const noexcept
    {
        return m_c2sSession->GetSessionID();
    }

    void NetworkMgr::FinishNetwork() noexcept
    {
        if (m_c2sSession->IsConnected())
        {
            m_c2sSession->Disconnect(L"Bye");
        }
        if (INVALID_SOCKET != m_c2sSession->m_sessionSocket)
        {
            shutdown(m_c2sSession->m_sessionSocket, SD_BOTH);
            SocketUtils::Close(m_c2sSession->m_sessionSocket);
        }
        WSACloseEvent(m_connectEvent);
        SocketUtils::Clear();
        m_c2sSession->ProcessDisconnect();
        m_c2sSession->m_pCacheSharedFromThis.reset();
    }

    bool NetworkMgr::Connect(std::wstring_view ip, uint16 port, const PacketHandleFunc* const handler) noexcept
    {
        m_serverAddr = NetAddress{ ip.data(),port };
        const auto limitTick = ::GetTickCount64() + 100000;
        bool bConnectSuccess = false;
        while (::GetTickCount64() < limitTick)
        {
            m_c2sSession = m_sessionFactory();
            m_c2sSession->m_serverAddr = m_serverAddr;
            m_c2sSession->m_sessionPacketHandler = handler;
            m_c2sSession->m_pCacheSharedFromThis = std::static_pointer_cast<PacketSession>(m_c2sSession->shared_from_this());

            if (!m_c2sSession->RegisterConnect())
            {
                m_c2sSession->m_pCacheSharedFromThis.reset();
                continue;
            }

            WSAEventSelect(m_c2sSession->GetSocket(), m_connectEvent, FD_CONNECT);
            WSANETWORKEVENTS networkEvents;

            const DWORD dwResult = WSAWaitForMultipleEvents(1, &m_connectEvent, FALSE, 10000, FALSE);
            if (dwResult == WSA_WAIT_FAILED)
            {
                NET_NAGOX_ASSERT_LOG(false, "WSA WAIT FAILED");
            }

            if (WSAEnumNetworkEvents(m_c2sSession->GetSocket(), m_connectEvent, &networkEvents) == SOCKET_ERROR)
            {
                NET_NAGOX_ASSERT_LOG(false, "Invalide Socket");
            }

            if (networkEvents.lNetworkEvents & FD_CONNECT &&
                0 == networkEvents.iErrorCode[FD_CONNECT_BIT] &&
                SOCKET_ERROR != setsockopt(m_c2sSession->GetSocket(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0) &&
                SocketUtils::SetTcpNoDelay(m_c2sSession->GetSocket(), true))
            {
                m_c2sSession->ProcessConnect();
                std::cout << "Server Join!" << std::endl;
                bConnectSuccess = true;
                break;
            }
        }
        if (bConnectSuccess)
        {
            WSAEventSelect(m_c2sSession->GetSocket(), m_connectEvent, FD_CLOSE);
            static SOCKET finSocket = m_c2sSession->GetSocket();
            std::atexit([]()noexcept
                {
                    shutdown(finSocket, SD_BOTH);
                    SocketUtils::Close(finSocket);
                    SocketUtils::Clear();
                });
        }
        return bConnectSuccess;
    }
}
