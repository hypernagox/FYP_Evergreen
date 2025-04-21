#include "pch.h"
#include "PartyQuestSystem.h"
#include "QuestRoom.h"
#include "ClientSession.h"
#include "Cluster.h"

bool PartyQuestSystem::MissionStart()
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	if (m_curQuestRoomInstance)return false;
	if (m_started)return false;
	if (m_runFlag)return false;
	if (-1 == m_curQuestID)return false;
	if (!CanMissionStart())return false;
	StartFlag();
	m_runFlag = true;
	if (0 == m_curQuestID)
	{
		m_curQuestRoomInstance = NagiocpX::MakeShared<FoxQuest>();
	}
	else
	{
		m_curQuestRoomInstance = NagiocpX::MakeShared<GoblinQuest>();
	}
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
	if (!m_runFlag) {
		std::cout << "Now not run\n";
		return;
	}
	// ���� 
	// m_runFlag = false; ���� ���⿡ �� �ڵ尡 �ִٸ�,
	// ���� ���� -> ���� ���� �� �� ������ ���� ���� ���� run flag�� �����, �� ���� ���⿡ ������
	// ���� ������ ����� �������� ���ߴµ� run flag�� true�ϱ�, ���� ����ϰԵȴ�.
	// �ٵ� ���� �ؿ� pending �˻縦 �Ѵٸ� ���⼭ Ż���� ���̰�, run flag�� false�� �ǰ� �� ���·� ������ ����Ϸ�
	// ��������� �δ� �ȿ� �����ߴµ� run flag�� false�� ��Ȳ�� �߻��ؼ� ����� Mission End�� ���� ���Ѵ�.
	// ���� ����˻����� ���� �ʴ´ٸ�, ���� Ŭ������ �����Ϳ�, ��ƼƼ ����ī��Ʈ�� �ڼ����� �������ϰԵȴ�
	// ��)      ť)    [�������� ������] - [�������� ������ ����] , �������� ������ ���� ���� ������� �������� ������ �����ϴ� ���� �Ұ��̰�
	// ���� �������ʾҴٸ� �������� ��� ���ֽõ����ٵ� ����� ������ �� ����.
	// ... ���� ���⿡ if ����˻� �־���.
	if (!CanMissionEnd())return;
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
	m_runFlag = false;
}

void PartyQuestSystem::ResetPartyQuestSystem()	
{
	std::lock_guard<std::mutex> lock{ m_partyLock };
	m_curQuestID = -1;
	for (auto& m : m_member)
	{
		if (!m)continue;
		m->m_cur_my_party_system.store(nullptr);
		m.reset();
	}
}

PARTY_ACCEPT_RESULT PartyQuestSystem::AcceptNewMember(S_ptr<ClientSession> new_member)
{
	XVector<S_ptr<ClientSession>> sessions; sessions.reserve(NUM_OF_MAX_PARTY_MEMBER - 1);
	PARTY_ACCEPT_RESULT res = PARTY_ACCEPT_RESULT::PARTY_IS_FULL;
	const auto target_user_id = new_member->GetSessionID();
	{
		std::lock_guard<std::mutex> lock{ m_partyLock };
		if (m_started)return PARTY_ACCEPT_RESULT::INVALID;
		for (int i = 1; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
		{
			if (m_member[i]) 
			{
				sessions.emplace_back(m_member[i]);
				continue;
			}
			m_member[i].swap(new_member);
			res = PARTY_ACCEPT_RESULT::ACCEPT_SUCCESS;
		}
	}
	for (const auto& other_player : sessions)
	{
		other_player->SendAsync(Create_s2c_PARTY_JOIN_NEW_PLAYER(target_user_id));
	}
	return res;
}

bool PartyQuestSystem::CanMissionStart() const noexcept
{
	for (const auto& session : m_member)
	{
		if (!session)continue;
		const auto entity = session->GetOwnerEntity();
		if (entity->IsPendingClusterEntry()) {
			std::cout << "Now Pending at start ...\n";
			return false;
		}
		if (-1 == entity->GetClusterFieldInfo().clusterInfo.fieldID) {
			std::cout << "Current Field is Party Quest, May be quest not finished ...\n";
			return false;
		}
	}
	return true;
}

bool PartyQuestSystem::CanMissionEnd() const noexcept
{
	for (const auto& session : m_member)
	{
		if (!session)continue;
		const auto entity = session->GetOwnerEntity();
		if (entity->IsPendingClusterEntry()) {
			std::cout << "Now Pending at end ...\n";
			return false;
		}
		if (-1 != entity->GetClusterFieldInfo().clusterInfo.fieldID) {
			std::cout << "Current Field is Not Party Quest\n";
			return false;
		}
	}
	return true;
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
