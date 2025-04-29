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

void PathNPC::UpdateMove()
{
	const auto cur_time = GetTickCount64();
	const auto owner = GetOwnerEntity();
	if (m_cur_idx == m_vecDirDists.size())
	{
		// µµÂø
		m_owner_system->m_curQuestRoomInstance->CheckPartyQuestState();
		std::cout << "Å¬¸®¾î\n";
		return;
	}
	
	m_curDistAcc += m_navAgent->ApplyPostPosition(m_vecDirDists[m_cur_idx].first, m_speed, (cur_time - m_last_update_timestamp) * 0.001f);

	if (m_curDistAcc >= m_vecDirDists[m_cur_idx].second)
	{
		m_curDistAcc = 0.f;
		++m_cur_idx;
	}
	
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
	const Vector3 begin = { -19.601448f,  72.97739f,  0.74976814f };
	const Vector3 end = { -119.499115f,75,13.64f }; // ¸¶À» Áß¾Ó
	const auto& vecPath = m_navAgent->GetAgentConcreate()->
		GetNavMesh()->GetPathVertices(begin, end, 1);
	m_navAgent->SetPos(begin);
	m_speed = 5.f;
	if (auto num = vecPath.size())
	{
		m_vecDirDists.reserve(--num);
		for (int i = 0; i < num; ++i)
		{
			m_vecDirDists.emplace_back(
				CommonMath::Normalized(vecPath[i + 1] - vecPath[i]),
				Vector3::Distance(vecPath[i], vecPath[i + 1])
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
