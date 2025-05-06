#pragma once
#include "Field.h"

struct PartyQuestInfo {
	uint32_t leader_id;
	int32_t quest_id;
};

enum class PARTY_ACCEPT_RESULT :int8_t {
	INVALID = -1,
	PARTY_IS_FULL = 0,
	ACCEPT_SUCCESS = 1
};

class QuestRoom
	:public NagiocpX::Field
{
public:
	QuestRoom()noexcept;
	virtual ~QuestRoom()noexcept;
private:
	virtual void InitFieldGlobal()noexcept override;
	virtual void InitFieldTLS()noexcept override;
	virtual void DestroyFieldTLS()noexcept override;

	virtual bool ProcessPartyQuest()noexcept = 0;
	// TODO: 실패조건
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept;
	virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept;
protected:
public:
	virtual void InitQuestField()noexcept = 0;
	virtual void MigrationAfterBehavior(Field* const prev_field)noexcept override;
	void DecMemberCount()noexcept;
	void IncMemberCount()noexcept { m_numOfMember.fetch_add(1); }
	const auto GetMemberCount()const noexcept { return m_numOfMember.load(); }
	void SetOwnerSystem(class PartyQuestSystem* sys) { m_ownerPartrySystem = sys; }
	const auto GetOwnerSystem()const noexcept { return m_ownerPartrySystem; }
public:
	void CheckPartyQuestState()noexcept;
	bool IsClear()const noexcept { return m_isClear.load(); }
	void RegisterMember(const uint32_t idx, ContentsEntity* const entity)noexcept;
protected:
	NagoxAtomic::Atomic<int8_t> m_monKillCount{ 0 };
private:
	NagoxAtomic::Atomic<bool> m_isClear{ false };
	NagoxAtomic::Atomic<int8_t> m_numOfMember{ 0 };
	class PartyQuestSystem* m_ownerPartrySystem = nullptr;
	ContentsEntity* m_members[NUM_OF_MAX_PARTY_MEMBER]{ nullptr };
	XMap<uint32_t, uint32_t> m_id2idx_table;
};

class FoxQuest
	:public QuestRoom
{
public:
	// TODO: 락 고려
	virtual bool ProcessPartyQuest()noexcept override{
		return 1 == m_monKillCount.fetch_add(1);
	}
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	//virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept override;
	//virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept override;
	virtual void InitQuestField()noexcept override;
private:
};

class GoblinQuest
	:public QuestRoom
{
public:
	virtual bool ProcessPartyQuest()noexcept override {
		return 1 == m_monKillCount.fetch_add(1);
	}
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	//virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept override;
	//virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept override;
	virtual void InitQuestField()noexcept override;
private:
};

class NPCGuardQuest
	:public QuestRoom
{
public:
	virtual bool ProcessPartyQuest()noexcept override {
		return true;
	}
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	//virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept override;
	//virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept override;
	virtual void InitQuestField()noexcept override;
};