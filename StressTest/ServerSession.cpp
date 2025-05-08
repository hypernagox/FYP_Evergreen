#include "pch.h"
#include "ServerSession.h"
#include "s2c_DummyPacketHandler.h"
#include "CreateBuffer4Dummy.h"
#include "Navigator.h"
#include "HarvestLoader.h"
#include "NaviAgent.h"

extern Vector3 O_VEC3(const Nagox::Struct::Vec3* const v);
extern Nagox::Struct::Vec3 F_VEC3(const Vector3& v);

static inline std::mt19937 rng{ std::random_device{}() };
static inline std::uniform_int_distribution<int> dist_harvest(0, 1023);

Vector3 ServerSession::GetKthNearestHarvestPoint(const size_t k) noexcept
{
	const Vector3 cur_pos = pos;
	const auto& infos = HarvestLoader::GetHarvestPos();

	XVector<std::pair<float, Vector3>> dist_list;
	dist_list.reserve(infos.size());
	for (const auto& info : infos)
	{
		const float dx = cur_pos.x - info.harvest_pos.x;
		const float dy = cur_pos.y - info.harvest_pos.y;
		const float dz = cur_pos.z - info.harvest_pos.z;
		const float distSq = dx * dx + dy * dy + dz * dz;
		dist_list.emplace_back(distSq, info.harvest_pos);
	}

	if (dist_list.empty())
	{
		return cur_pos;
	}

	const size_t idx = (dist_list.size() > k ? k : dist_list.size() - 1);

	std::ranges::nth_element(
		dist_list.begin(),
		dist_list.begin() + idx,
		dist_list.end(),
		[](const auto& a, const auto& b)noexcept { return a.first < b.first; }
	);

	return dist_list[idx].second;
}

ServerSession::ServerSession()
	:NagiocpX::PacketSession{ true }
{
}

ServerSession::~ServerSession()
{
}

void ServerSession::OnConnected()
{
	std::cout << "Im Dummy !" << std::endl;
	SendAsync(Create_c2s_LOGIN("Hello"));
}

void ServerSession::OnSend(c_int32 len)noexcept
{
}

void ServerSession::OnDisconnected(const NagiocpX::Cluster* const curCluster_)noexcept
{
	IncRef();
	std::cout << "DisConnect !" << std::endl;
}

void ServerSession::StartMovePacket() noexcept
{
	SetPath();

	UpdateMove();

	SendAsync(Create_c2s_MOVE(F_VEC3(pos), F_VEC3(vel), {}, {}, ::GetTickCount64()));

	Mgr(TaskTimerMgr)->ReserveAsyncTask(delay_time, &ServerSession::SendMovePacketRoutine, SharedFromThis<ServerSession>());
}

Vector3 ServerSession::GetNearestHarvestPoint() noexcept
{
	const auto cur_pos = pos;
	Vector3 nearest = cur_pos;
	float minDistSq = std::numeric_limits<float>::max();
	{
		std::scoped_lock lk{ m_lk };
		for (const auto& info : m_h)
		{
			if (!info.second.second)continue;
			const float dx = cur_pos.x - info.second.first.x;
			const float dy = cur_pos.y - info.second.first.y;
			const float dz = cur_pos.z - info.second.first.z;
			const float distSq = dx * dx + dy * dy + dz * dz;

			if (distSq < minDistSq)
			{
				minDistSq = distSq;
				nearest = info.second.first;
			}
		}
	}
	if (nearest == cur_pos)
	{
		nearest = GetKthNearestHarvestPoint(dist_harvest(rng) % 10);
	}

	return nearest;
}

void ServerSession::SendMovePacketRoutine() noexcept
{
	UpdateMove();
	//CommonMath::GetYawFromQuaternion()

	constexpr const auto GetYawDeg = [](const Vector3& dir) noexcept
		{
			constexpr float RAD2DEG = 180.0f / 3.14159265358979323846f;
			return std::atan2(dir.x, dir.z) * RAD2DEG;
		};

	SendAsync(Create_c2s_MOVE(F_VEC3(pos), F_VEC3(vel), F_VEC3(accel), GetYawDeg(vel), ::GetTickCount64()));

	if (m_moveCount % m_print_count == 0 && rand() & 1)
	{
		std::cout << std::format("ID: {}, Delay Avg: {:.3f}ms \n", m_id, GetDelayAvg());
	}

	Mgr(TaskTimerMgr)->ReserveAsyncTask(delay_time, &ServerSession::SendMovePacketRoutine, SharedFromThis<ServerSession>());
}


void ServerSession::UpdateMove() noexcept
{
	++m_moveCount;
	//pos += dir * 4.f * 0.5f;
	
	NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(pos);

	//++dir_count;
	//if (10 < dir_count)
	//{
	//	vel = dir * 4.f;
	//	dir_count = 0;
	//	dir = pickRandomDir8();
	//}

	const auto cur_time = GetTickCount64();
	const auto owner = GetOwnerEntity();
	const auto dt = (cur_time - m_last_update_timestamp) * 0.001f;

	if (m_cur_idx == m_vecDirDists.size())
	{
		// 도착
		vel = {};
		accel = {};
		NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(pos);
		SendAsync(Create_c2s_CHANGE_HARVEST_STATE());
		SendAsync(Create_c2s_ACQUIRE_ITEM(0));
		SetPath();
		return;
	}

	const auto dir_ = CommonMath::Normalized(m_vecDirDists[m_cur_idx].first) * 4.f * dt;
	vel = m_vecDirDists[m_cur_idx].first;
	accel = vel;
	pos += dir_;

	NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(pos);

	
	m_curDistAcc += dir_.Length();
	//m_curDistAcc += m_navAgent->ApplyPostPosition(m_vecDirDists[m_cur_idx].first, m_speed, (cur_time - m_last_update_timestamp) * 0.001f);

	//owner->GetComp<PositionComponent>()->vel = m_vecDirDists[m_cur_idx].first;
	//owner->GetComp<PositionComponent>()->accel = {};

	if (m_curDistAcc >= m_vecDirDists[m_cur_idx].second)
	{
		m_curDistAcc = 0.f;
		++m_cur_idx;
	}

	//owner->GetComp<NagiocpX::ClusterInfoHelper>()->AdjustCluster()
	
	m_last_update_timestamp = cur_time;
	// 1. 길 찾기,
	// 2. Path NPC로직 사용
	// 3. 주기적으로 그거 체크하면서 있나없나
	// 4. 사거리안이라면 F키 누르기 패킷
	// 5. 먹었거나 남이 먹었으면 다른 곳으로
	// TODO: 적당히 랜덤무브
}

void ServerSession::SetPath() noexcept
{
	m_curDistAcc = 0.f;
	m_cur_idx = 0;
	m_vecDirDists.clear();

	const float step = 5.f;

	NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(pos);
	
	const auto& v = HarvestLoader::GetHarvestPos();
	const Vector3 end = GetKthNearestHarvestPoint(dist_harvest(rng) % 10);

	const auto& vecPath = NAVIGATION->GetNavMesh(NUM_0)->GetPathVertices(pos, end, step);


	m_speed = 5.f;

	if (auto num = vecPath.size())
	{
		m_vecDirDists.reserve(--num);
		for (int i = 0; i < num; ++i)
		{
			m_vecDirDists.emplace_back(
				CommonMath::Normalized(vecPath[i + 1] - vecPath[i]),
				std::min(Vector3::Distance(vecPath[i], vecPath[i + 1]), step)
			);
		}
		m_last_update_timestamp = GetTickCount64();
		//UpdateMove();
	}
	else
	{
		// Invalid Path ... 
	}
}
