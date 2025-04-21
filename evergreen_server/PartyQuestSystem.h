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

	// TODO: 레디 상태 등등 확인 후 인스턴스 생성과 함께 시작
	bool MissionStart();

	// TODO: 퀘스트 종료시 원래 월드로 보내고 기존월드의 레퍼런스 카운터를 깐다.
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
	// TODO: 강퇴인지 자발적 퇴장인지 및, 해당 패킷필요함
public:
	S_ptr<ClientSession> FindMember(const uint32_t obj_id);
	void KickMember(const uint32_t obj_id);
public:
	// -1은 퀘스트가 없는 상태
	std::mutex m_partyLock;
	int m_curQuestID = -1;
	bool m_started = false;
	bool m_runFlag = false;
	NagiocpX::Field* m_prev_field = nullptr;
	S_ptr<QuestRoom> m_curQuestRoomInstance = nullptr;
	// 0번이 반드시 파티장
	S_ptr<ClientSession> m_member[NUM_OF_MAX_PARTY_MEMBER]{ nullptr };
};

