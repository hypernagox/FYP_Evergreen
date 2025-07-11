#include "pch.h"
#include "Receiver.h"
#include "ThreadMgr.h"
#include "../DBContentsPacket.hpp"

Receiver::Receiver()
{
}

Receiver::~Receiver()
{
    closesocket(m_mainServerSocket);

    closesocket(m_queryServerSocket);

    WSACleanup();
}

bool Receiver::Start(const std::wstring_view ip, const uint16_t port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;
    m_queryServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == m_queryServerSocket)
        return false;
    m_netAddr = NetAddress{ ip,port };
    const auto& addr = m_netAddr.GetSockAddr();
    if (SOCKET_ERROR == bind(m_queryServerSocket, (const sockaddr* const)&addr, sizeof(addr)))
        return false;
    if (SOCKET_ERROR == listen(m_queryServerSocket, SOMAXCONN))
        return false;

    std::cout << "Wait Main Server ..." << std::endl;

    m_mainServerSocket = accept(m_queryServerSocket, nullptr, nullptr);
    if (INVALID_SOCKET == m_mainServerSocket)
        return false;

    return true;
}

void Receiver::DoRecv()noexcept
{
    for (;;)
    {
        const int bytes_received = recv(m_mainServerSocket, reinterpret_cast<char*>(m_recvBuffer.WritePos()), m_recvBuffer.FreeSize(), 0);

        if (bytes_received == SOCKET_ERROR) [[unlikely]]
        {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
        if (bytes_received == 0) [[unlikely]]
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        OnRecv(bytes_received);
    }
}

void Receiver::OnRecv(const int32_t numofBytes_) noexcept
{
    if (0 == numofBytes_)
    {
        return;
    }

    if (false == m_recvBuffer.OnWrite(numofBytes_)) [[unlikely]]
    {
        return;
    }
    const int rsize = ProcessDBPacket(m_recvBuffer.ReadPos(), m_recvBuffer.DataSize());
    m_recvBuffer.OnRead(rsize);
    m_recvBuffer.Clear();
}

const int32_t Receiver::ProcessDBPacket(BYTE* const buffer, c_int32 len) noexcept
{
    int32 processLen = 0;

    for (;;)
    {
        const int32 dataSize = len - processLen;

        if (dataSize < static_cast<c_int32>(sizeof(DBContentsPacketHeader)))
            break;

        const DBContentsPacketHeader* const __restrict header = reinterpret_cast<const DBContentsPacketHeader* const>(buffer + processLen);
        c_uint16 packetSize = header->pkt_size;
        c_uint16 packetId = header->pkt_id;

        if (dataSize < packetSize)
            break;

        Mgr(ThreadMgr)->EnqueueDBPacket(packetId, { (BYTE* const)header,(BYTE* const)header + packetSize });

        processLen += packetSize;
    }

    return processLen;
}
