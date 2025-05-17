#include "pch.h"
#include "PathNPC.h"
#include "NavigationMesh.h"
#include "NaviAgent_Common.h"
#include "MoveBroadcaster.h"
#include "Cluster.h"
#include "Queueabler.h"
#include "PositionComponent.h"
#include "PartyQuestSystem.h"
#include "QuestRoom.h"
#include "ClientSession.h"
#include "ClusterInfoHelper.h"

void PathNPC::UpdateMove()
{
	const auto cur_time = GetTickCount64();
	const auto owner = GetOwnerEntity();
	if (m_cur_idx == m_vecDirDists.size())
	{
		// 도착
		// TODO 릭 가능성
		if (m_owner_system->m_curQuestRoomInstance) {
			static_cast<NPCGuardQuest*>(
				m_owner_system->m_curQuestRoomInstance.get()
				)->m_clear.store(true);
			m_owner_system->m_curQuestRoomInstance->CheckPartyQuestState();
		}
		std::cout << "클리어\n";
		if(m_owner_system_session)
			m_owner_system_session->DecRef();
		return;
	}
	if (const auto leader = m_owner_system_session)
	{
		const auto leader_pos = leader->GetComp<PositionComponent>()->pos;
		const auto my_pos = owner->GetComp<PositionComponent>()->pos;
		if (!CommonMath::IsInDistanceDX(leader_pos, my_pos, 10))
		{
			m_last_update_timestamp = cur_time;
			owner->GetQueueabler()->EnqueueAsyncTimer(
				100,
				&PathNPC::UpdateMove,
				this
			);
			return;
		}
	}
	m_curDistAcc += m_navAgent->ApplyPostPosition(m_vecDirDists[m_cur_idx].first, m_speed, (cur_time - m_last_update_timestamp) * 0.001f);

	//owner->GetComp<PositionComponent>()->vel = m_vecDirDists[m_cur_idx].first;
	//owner->GetComp<PositionComponent>()->accel = {};

	if (m_curDistAcc >= m_vecDirDists[m_cur_idx].second)
	{
		m_curDistAcc = 0.f;
		++m_cur_idx;
	}
	
	//owner->GetComp<NagiocpX::ClusterInfoHelper>()->AdjustCluster()
	owner->GetCurCluster()->Broadcast(
		NagiocpX::MoveBroadcaster::CreateMovePacket(owner)
	);

	m_last_update_timestamp = cur_time;

	owner->GetQueueabler()->EnqueueAsyncTimer(
		100,
		&PathNPC::UpdateMove,
		this
	);
}

void PathNPC::InitPathNPC()
{
	const float step = 2.f;
	const Vector3 begin = Vector3(-270.50497F, 86.48416F, -23.966377F);
	//const Vector3 end = Vector3(-120.628365, 75.83887, 12.696598); // 마을 중앙
	const Vector3 points[] = {
	Vector3(-270.50497F,86.48416F,-23.966377F),
	Vector3(-262.50986F,86.0128F,-23.310076F),
	Vector3(-248.07124F,84.52976F,-15.239957F),
	Vector3(-236.64597F,83.31353F,-5.343036F),
	Vector3(-229.64525F,82.18539F,-2.87825F),
	Vector3(-209.29497F,81.04176F,-3.3852773F),
	Vector3(-188.42654F,79.49546F,-3.2104106F),
	Vector3(-169.08516F,78.3188F,-3.3492434F),
	Vector3(-149.05505F,78.25525F,-0.90608793F),
	Vector3(-139.2348F,78.220726F,-0.37422684F),
	Vector3(-122.58272F,75.91813F,11.575291F),
	};
	const auto num = sizeof(points) / sizeof(points[0]);
	std::vector<Vector3> pvec;
	for (int i=0;i<num-1;++i)
	{
		const auto& vecPath = m_navAgent->GetAgentConcreate()->
			GetNavMesh()->GetPathVertices(points[i], points[i + 1], step);
		for (const auto& v : vecPath) {
			pvec.emplace_back(v);
		}
	}
	//const auto& vecPath = m_navAgent->GetAgentConcreate()->
	//	GetNavMesh()->GetPathVertices(begin, end, step);
	m_navAgent->SetPos(points[0]);
	m_speed = 5.f;

	if (const auto owner_session = m_owner_system->GetPartyLeader())
	{
		m_owner_system_session = owner_session;
	}
	if (auto num = pvec.size())
	{
		m_vecDirDists.reserve(--num);
		for (int i = 0; i < num; ++i)
		{
			m_vecDirDists.emplace_back(
				CommonMath::Normalized(pvec[i + 1] - pvec[i]),
				std::min(Vector3::Distance(pvec[i], pvec[i + 1]),step)
			);
		}
		m_last_update_timestamp = GetTickCount64();
		UpdateMove();
	}
	else
	{
		// Invalid Path ... 
	}
}
