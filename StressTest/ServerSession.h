#pragma once
#include "PacketSession.h"

class ServerSession
	:public ServerCore::PacketSession
{
public:
	ServerSession();
	~ServerSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override;
	virtual void OnDisconnected(const ServerCore::Cluster* const curCluster_)noexcept override;

	const auto GetDelayAvg()const noexcept { return ((float)m_accDelayMs / (float)m_moveCount); }
	void UpdateTimeStamp(const uint64_t old_time_stamp)noexcept { m_accDelayMs += (::GetTickCount64() - old_time_stamp); }
public:
	void StartMovePacket(const uint64_t delay_time = 500)noexcept;
private:
	void SendMovePacketRoutine(const uint64_t delay_time)noexcept;
	void UpdateMove()noexcept;
public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	uint64_t m_id = 0;
	uint64_t m_moveCount = 0;
	uint64_t m_accDelayMs = 0;
	const uint64_t m_print_count = 10 + (rand() % 1000);
private:

};

