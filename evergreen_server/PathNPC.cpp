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
		if(m_owner_system->m_curQuestRoomInstance)
			m_owner_system->m_curQuestRoomInstance->CheckPartyQuestState();
		std::cout << "클리어\n";
		if(m_owner_system_session)
			m_owner_system_session->DecRef();
		return;
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
		1000,
		&PathNPC::UpdateMove,
		this
	);
}

void PathNPC::InitPathNPC()
{
	const float step = 5.f;
	const Vector3 begin = { -19.601448f,  72.97739f,  0.74976814f };
	const Vector3 end = { -119.499115f,75,13.64f }; // 마을 중앙
	const auto& vecPath = m_navAgent->GetAgentConcreate()->
		GetNavMesh()->GetPathVertices(begin, end, step);
	m_navAgent->SetPos(begin);
	m_speed = 5.f;

	if (const auto owner_session = m_owner_system->m_member[0])
	{
		owner_session->IncRef();
		m_owner_system_session = owner_session;
	}
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
		UpdateMove();
	}
	else
	{
		// Invalid Path ... 
	}
}
