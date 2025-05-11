#pragma once
#include "PacketSession.h"

class ServerSession
	:public NagiocpX::PacketSession
{
public:
	ServerSession();
	~ServerSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)noexcept override;
	virtual void OnDisconnected(const NagiocpX::Cluster* const curCluster_)noexcept override;

	const auto GetDelayAvg()const noexcept { return ((float)m_accDelayMs / (float)m_moveCount); }
	void UpdateTimeStamp(const uint64_t old_time_stamp)noexcept { m_accDelayMs += (::GetTickCount64() - old_time_stamp); }
public:
	void StartMovePacket()noexcept;
	Vector3 GetNearestHarvestPoint()noexcept;

	void UpdateHarvest(const uint32_t id, const Vector3& pos, const  bool is_active) {
		std::scoped_lock lk{ m_lk };
		m_h[id] = std::make_pair(pos, is_active);
	}
	void UpdateHarvest(const uint32_t id,const  bool is_active) {
		std::scoped_lock lk{ m_lk };
		if (!m_h.contains(id))return;
		m_h[id].second = is_active;
	}
	void SetPath()noexcept;
	Vector3 GetKthNearestHarvestPoint(const size_t k) noexcept;
private:
	void SendMovePacketRoutine()noexcept;
	void UpdateMove()noexcept;
	

public:
	Vector3 pos;
	Vector3 vel;
	Vector3 accel;
	Vector3 dir = {};
	uint64_t m_id = 0;
	uint64_t m_moveCount = 0;
	uint64_t m_accDelayMs = 0;
	const uint64_t m_print_count = 10 + (rand() % 1000);

	uint64_t dir_count = 0;
	uint64_t delay_time = 1000;

public:
	XVector<std::pair<Vector3, float>> m_vecDirDists;
	int m_cur_idx = 0;
	float m_speed = 1.f;
	float m_curDistAcc = 0.f;
	class PartyQuestSystem* m_owner_system = nullptr;
	uint64_t m_last_update_timestamp = ::GetTickCount64();
	ClientSession* m_owner_system_session = nullptr;


	XHashMap<uint32_t, std::pair<Vector3, bool>> m_h;
	std::mutex m_lk;
private:

};

