#include "pch.h"
#include "PartyQuestSystem.h"
#include "QuestRoom.h"
#include "ClientSession.h"
#include "Cluster.h"

void PartyQuestSystem::MissionStart()
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	if (m_started)return;
	if (-1 == m_curQuestID)return;
	StartFlag();
	m_curQuestRoomInstance = NagiocpX::MakeShared<QuestRoom>();
	m_curQuestRoomInstance->tempid = m_curQuestID;
	m_curQuestRoomInstance->InitQuestField();
	m_prev_field =
		m_member[0]->GetOwnerEntity()->GetClusterFieldInfo().curFieldPtr;
	m_curQuestRoomInstance->SetOwnerSystem(this);
	for (const auto& session : m_member)
	{
		if (!session)continue;
		m_curQuestRoomInstance->IncMemberCount();
		const auto owner = session->GetOwnerEntity();
		owner->GetCurCluster()->MigrationOtherFieldEnqueue(
			m_curQuestRoomInstance.get(),
			owner
		);
	}
}

void PartyQuestSystem::MissionEnd()
{
	m_curQuestRoomInstance->FinishField();
	std::lock_guard<std::mutex> lock{ m_partyLock };
	for (auto& session : m_member)
	{
		if (!session)continue;
		const auto owner = session->GetOwnerEntity();
		owner->GetCurCluster()->MigrationOtherFieldEnqueue(
			m_prev_field,
			owner
		);
	}
	m_curQuestRoomInstance.reset();
}

void PartyQuestSystem::ResetPartyQuestSystem()
{
	m_curQuestID = -1;
	for (auto& m : m_member)
	{
		if (!m)continue;
		m->m_cur_my_party_system.store(nullptr);
		m.reset();
	}
}

S_ptr<ClientSession> PartyQuestSystem::FindMember(const uint32_t obj_id)
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
	{
		if (!m_member[i])continue;
		if (m_member[i]->GetSessionID() == obj_id)
		{
			return m_member[i];
		}
	}
	return {};
}

void PartyQuestSystem::KickMember(const uint32_t obj_id)
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
	{
		if (!m_member[i])continue;
		if (m_member[i]->GetSessionID() == obj_id)
		{
			m_member[i]->m_cur_my_party_system.store_relaxed(nullptr);
			m_member[i].reset();
			return;
		}
	}
}
