#include "pch.h"
#include "PartyQuestSystem.h"
#include "QuestRoom.h"
#include "ClientSession.h"
#include "Cluster.h"
#include "PositionComponent.h"

PartyQuestSystem::~PartyQuestSystem() noexcept
{
}

bool PartyQuestSystem::MissionStart()
{
	NagiocpX::SRWLockGuardEx lock{ m_partyLock };
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
	else if (2 == m_curQuestID)
	{
		m_curQuestRoomInstance = NagiocpX::MakeShared<NPCGuardQuest>();
	}
	else
	{
		m_curQuestRoomInstance = NagiocpX::MakeShared<GoblinQuest>();
	}
	m_curQuestRoomInstance->SetOwnerSystem(this);
	m_curQuestRoomInstance->InitQuestField();
	m_prev_field =
		m_member[0]->GetClusterFieldInfo().curFieldPtr;
	for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
	{
		const auto owner = m_member[i].get();
		// TODO: ������ġ
		if (!owner)continue;
		const auto session = owner->GetClientSession();
		if (!session)continue;
		m_curQuestRoomInstance->IncMemberCount();
		const auto pos = owner->GetComp<PositionComponent>()->pos;
		//m_curQuestRoomInstance->RegisterMember(i, owner);
		owner->GetCurCluster()->MigrationOtherFieldEnqueue(
			m_curQuestRoomInstance.get(),
			owner,
			m_curQuestRoomInstance->CalculateClusterXY(pos.x + 512.f, pos.z + 512.f)
		);
	}
	return true;
}

void PartyQuestSystem::MissionEnd()
{
	NagiocpX::SRWLockGuardEx lock{ m_partyLock };
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
	for (const auto& mem : m_member)
	{
		// TODO: ��ȯ��ġ
		if (!mem)continue;
		const auto owner = mem.get();
		const auto session = owner->GetClientSession();
		const auto pos = owner->GetComp<PositionComponent>()->pos;
		owner->GetCurCluster()->MigrationOtherFieldEnqueue(
			m_prev_field,
			owner,
			m_prev_field->CalculateClusterXY(pos.x + 512.f, pos.z + 512.f)
		);
	}
	m_curQuestRoomInstance.reset();
	m_runFlag = false;
}

void PartyQuestSystem::ResetPartyQuestSystem()	
{
	NagiocpX::SRWLockGuardEx lock{ m_partyLock };
	m_curQuestID = -1;
	for (auto& m : m_member)
	{
		if (!m)continue;
		m->GetClientSession()->m_cur_my_party_system.store(nullptr);
		m = nullptr;
	}
	m_curQuestID = -1;
	m_prev_field = nullptr;
	m_curQuestRoomInstance.reset();
	m_started = m_runFlag = false;
}

PARTY_ACCEPT_RESULT PartyQuestSystem::AcceptNewMember(S_ptr<ContentsEntity> new_member)
{
	XVector<S_ptr<ContentsEntity>> sessions; sessions.reserve(NUM_OF_MAX_PARTY_MEMBER - 1);
	XVector<uint32_t> other_member_ids; sessions.reserve(NUM_OF_MAX_PARTY_MEMBER - 1);
	const auto new_mem = new_member;
	PARTY_ACCEPT_RESULT res = PARTY_ACCEPT_RESULT::PARTY_IS_FULL;
	const auto target_user_id = new_member->GetObjectID();
	{
		NagiocpX::SRWLockGuardEx lock{ m_partyLock };
		if (m_started)return PARTY_ACCEPT_RESULT::INVALID;
		for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
		{
			if (m_member[i]) 
			{
				sessions.emplace_back(m_member[i]);
				continue;
			}
			std::swap(new_member.m_count_ptr, m_member[i].m_count_ptr);
			//m_member[i].swap(new_member);
			res = PARTY_ACCEPT_RESULT::ACCEPT_SUCCESS;
			break;
		}
	}
	if (!sessions.empty())
	{
		for (const auto& other_player : sessions)
		{
			other_player->GetSession()->SendAsync(Create_s2c_PARTY_JOIN_NEW_PLAYER(target_user_id));
			other_member_ids.emplace_back(other_player->GetObjectID());
		}
		new_mem->GetSession()->SendAsync(Create_s2c_PARTY_MEMBERS_INFORMATION(std::move(other_member_ids)));
	}
	return res;
}

bool PartyQuestSystem::CanMissionStart() const noexcept
{
//	NagiocpX::SRWLockGuard lock{ m_partyLock };
	for (const auto& mem : m_member)
	{
		if (!mem)continue;
		const auto entity = mem.get();
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
	{
		//NagiocpX::SRWLockGuard lock{ m_partyLock };
		for (const auto& mem : m_member)
		{
			if (!mem)continue;
			const auto entity = mem.get();
			if (entity->IsPendingClusterEntry()) {
				std::cout << "Now Pending at end ...\n";
				return false;
			}
			if (-1 != entity->GetClusterFieldInfo().clusterInfo.fieldID) {
				std::cout << "Current Field is Not Party Quest\n";
				return false;
			}
		}
	}
	return true;
}

S_ptr<ContentsEntity> PartyQuestSystem::FindMember(const uint32_t obj_id)
{
	{
		NagiocpX::SRWLockGuard lock{ m_partyLock };
		for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
		{
			if (!m_member[i])continue;
			if (m_member[i]->GetObjectID() == obj_id)
			{
				return m_member[i];
			}
		}
	}
	return {};
}

void PartyQuestSystem::OutMember(const uint32_t obj_id)
{
	XVector<S_ptr<ContentsEntity>> sessions; sessions.reserve(NUM_OF_MAX_PARTY_MEMBER);
	bool is_leader = false;
	{
		NagiocpX::SRWLockGuardEx lock{ m_partyLock };
		//if (m_started)return;
		//if (m_runFlag)return;	
		for (int i = 0; i < NUM_OF_MAX_PARTY_MEMBER; ++i)
		{
			if (!m_member[i])continue;
			if (m_member[i]->GetObjectID() == obj_id)
			{
				if (0 == i)is_leader = true;
				m_member[i]->GetClientSession()
					->m_cur_my_party_system.store(nullptr);
				sessions.emplace_back(m_member[i]);
				m_member[i] = nullptr;
				//m_member[i].reset();
			}
			else
			{
				sessions.emplace_back(m_member[i]);
				if (!m_member[0] && 0 != i)
				{
					m_member[0].swap(m_member[i]);
				}
			}
		}
		if (is_leader)
		{
			// ��Ƽ�����ٲ۴�.
			//ResetPartyQuestSystem();
			if (sessions.size() <= 1)
			{
				// ����

			}
			//else
			//{
			//	m_member[0] = sessions[1];
			//}
		}
	}
	auto pkt = Create_s2c_PARTY_OUT(obj_id, is_leader);
	for (const auto& remain_player : sessions)
	{
		remain_player->GetSession()->SendAsync(pkt);
	}
}

bool PartyQuestSystem::QueryPartyLeader(ContentsEntity* const owner) const noexcept
{
	NagiocpX::SRWLockGuard lock{ m_partyLock };
	return m_member[0].get() == owner;
}
