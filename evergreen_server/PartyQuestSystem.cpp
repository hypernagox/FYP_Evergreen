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
	// 주의 
	// m_runFlag = false; 만약 여기에 이 코드가 있다면,
	// 기존 마을 -> 던전 입장 할 때 던전에 입장 시작 직후 run flag를 세우고, 그 다음 여기에 들어오면
	// 아직 던전에 제대로 입장조차 못했는데 run flag는 true니까, 여길 통과하게된다.
	// 근데 만약 밑에 pending 검사를 한다면 여기서 탈락할 것이고, run flag는 false가 되고 그 상태로 던전에 입장완료
	// 결과적으로 인던 안에 입장했는데 run flag가 false인 상황이 발생해서 절대로 Mission End가 되지 못한다.
	// 또한 펜딩검사조차 하지 않는다면, 현재 클러스터 포인터와, 엔티티 입장카운트가 뒤섞여서 오동작하게된다
	// 예)      큐)    [던전으로 이주중] - [던전에서 마을로 이주] , 던전으로 이주중 일이 끝난 스레드는 던전에서 마을로 이주하는 일을 할것이고
	// 아직 끝나지않았다면 던전으로 계속 이주시도할텐데 결과를 예측할 수 없다.
	// ... 과거 여기에 if 펜딩검사 있었다.
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
