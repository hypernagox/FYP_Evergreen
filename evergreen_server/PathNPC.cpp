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
	const auto owner = GetOwnerEntity();
	const float dt = 1.f;
	if (m_cur_idx == m_vecPathDir.size())
	{
		// µµÂø
		m_owner_system->m_curQuestRoomInstance->CheckPartyQuestState();
		std::cout << "Å¬¸®¾î\n";
		return;
	}
	const auto pos_comp = owner->GetComp<PositionComponent>();

	m_curDistAcc += m_navAgent->ApplyPostPosition(m_vecPathDir[m_cur_idx], m_speed, dt);
	if (m_curDistAcc >= m_dists[m_cur_idx])
	{
		m_curDistAcc = 0.f;
		++m_cur_idx;
	}
	
	owner->GetCurCluster()->Broadcast(
		NagiocpX::MoveBroadcaster::CreateMovePacket(owner)
	);
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
	const auto num = vecPath.size() - 1;
	m_vecPathDir.reserve(num);
	m_dists.reserve(num);
	for (int i = 0; i < num; ++i)
	{
		m_vecPathDir.emplace_back(CommonMath::Normalized(vecPath[i + 1] - vecPath[i]));
		m_dists.emplace_back(Vector3::Distance(vecPath[i], vecPath[i + 1]));
	}
	UpdateMove();
}
