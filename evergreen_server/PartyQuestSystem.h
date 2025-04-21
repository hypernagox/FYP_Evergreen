#pragma once
#include "QuestRoom.h"
class ClientSession;
class QuestRoom;

struct PartyQuestInfo {
	uint32_t leader_id;
	int32_t quest_id;
};

enum class PARTY_ACCEPT_RESULT :int8_t {
	INVALID = -1,
	PARTY_IS_FULL = 0,
	ACCEPT_SUCCESS = 1
};

class PartyQuestSystem
{
	friend class ClientSession;
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
	const auto GetPartyLeader()const noexcept { return m_member[0].get(); }
	void StartFlag() { m_started = true; }
	void EndFlag() { m_started = false; }
private:
	PARTY_ACCEPT_RESULT AcceptNewMember(S_ptr<ClientSession> new_member);
	bool CanMissionStart()const noexcept;
	bool CanMissionEnd()const noexcept;
	// TODO: �������� �ڹ��� �������� ��, �ش� ��Ŷ�ʿ���
public:
	S_ptr<ClientSession> FindMember(const uint32_t obj_id);
	void KickMember(const uint32_t obj_id);
public:
	// -1�� ����Ʈ�� ���� ����
	std::mutex m_partyLock;
	int m_curQuestID = -1;
	bool m_started = false;
	bool m_runFlag = false;
	NagiocpX::Field* m_prev_field = nullptr;
	S_ptr<QuestRoom> m_curQuestRoomInstance = nullptr;
	// 0���� �ݵ�� ��Ƽ��
	S_ptr<ClientSession> m_member[NUM_OF_MAX_PARTY_MEMBER]{ nullptr };
};

