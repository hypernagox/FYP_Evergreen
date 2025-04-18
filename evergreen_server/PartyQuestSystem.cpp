#include "pch.h"
#include "PartyQuestSystem.h"
#include "QuestRoom.h"
#include "ClientSession.h"
#include "Cluster.h"

bool PartyQuestSystem::MissionStart()
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	const auto cur_time = ::GetTickCount64();
	const auto diff = cur_time - m_time_stamp;
	if (diff < PARTY_COMMAND_COOL_TIME)return false;
	m_time_stamp = cur_time;
	if (m_curQuestRoomInstance) {
		return false;
	}
	if (m_started)return false;
	if (m_runFlag)return false;
	if (-1 == m_curQuestID)return false;
	if (m_member[0]->GetOwnerEntity()->IsPendingClusterEntry()) {
		std::cout << "Now Pending ...\n";
		return false;
	}
	StartFlag();
	m_runFlag = true;
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
	return true;
}

void PartyQuestSystem::MissionEnd()
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	const auto cur_time = ::GetTickCount64();
	const auto diff = cur_time - m_time_stamp;
	if (diff < PARTY_COMMAND_COOL_TIME)return;
	//m_time_stamp = cur_time;
	if (!m_runFlag)return;
	m_runFlag = false;
	if (!m_curQuestRoomInstance)return;
	if (m_member[0]->GetOwnerEntity()->IsPendingClusterEntry()) {
		std::cout << "Now Pending ...\n";
		return;
	}
	m_curQuestRoomInstance->FinishField();
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
