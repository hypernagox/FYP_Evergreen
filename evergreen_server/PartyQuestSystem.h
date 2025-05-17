#pragma once
#include "QuestRoom.h"

class ClientSession;
class QuestRoom;

class PartyQuestSystem
{
	friend class ClientSession;
public:
	~PartyQuestSystem()noexcept;
public:
	void SetTargetQuest(const int quest_id) { m_curQuestID = quest_id; }
	void ResetQuestID() { SetTargetQuest(-1); }

	// TODO: ���� ���� ��� Ȯ�� �� �ν��Ͻ� ������ �Բ� ����
	bool MissionStart();

	// TODO: ����Ʈ ����� ���� ����� ������ ���������� ���۷��� ī���͸� ���.
	void MissionEnd();
public:
	void ResetPartyQuestSystem();
public:
	const auto GetPartyLeader()const noexcept { 
		m_partyLock.lock_shared();
		auto leader_ptr = m_member[0];
		m_partyLock.unlock_shared();
		return leader_ptr;
	}
	void StartFlag() { m_started = true; }
	void EndFlag() { m_started = false; }
private:
	PARTY_ACCEPT_RESULT AcceptNewMember(S_ptr<ContentsEntity> new_member);
	bool CanMissionStart()const noexcept;
	bool CanMissionEnd()const noexcept;
public:
	S_ptr<ContentsEntity> FindMember(const uint32_t obj_id);
	void OutMember(const uint32_t obj_id);
	bool QueryPartyLeader(ContentsEntity* const owner)const noexcept;
public:
	// -1�� ����Ʈ�� ���� ����
	mutable NagiocpX::SRWLock m_partyLock;
	int m_curQuestID = -1;
	bool m_started = false;
	bool m_runFlag = false;
	NagiocpX::Field* m_prev_field = nullptr;
	S_ptr<QuestRoom> m_curQuestRoomInstance = nullptr;
	// 0���� �ݵ�� ��Ƽ��
	S_ptr<ContentsEntity> m_member[NUM_OF_MAX_PARTY_MEMBER]{ nullptr };
};

