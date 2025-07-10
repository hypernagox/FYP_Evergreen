#pragma once
#include "NetAddress.h"
#include "RecvBuffer.h"

class Receiver
	:public Singleton<Receiver>
{
	friend class Singleton;
	Receiver();
	~Receiver();
public:
	bool Start(const std::wstring_view ip, const uint16_t port);
	void DoRecv()noexcept;
private:
	void OnRecv(const int32_t numofBytes_)noexcept;
	static const int32_t ProcessDBPacket(BYTE* const buffer, c_int32 len)noexcept;
private:
	SOCKET m_mainServerSocket = INVALID_SOCKET;
	RecvBuffer m_recvBuffer{ RecvBuffer::BUFFER_SIZE };
	SOCKET m_queryServerSocket = INVALID_SOCKET;
	NetAddress m_netAddr;
};

