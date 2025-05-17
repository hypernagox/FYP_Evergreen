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

class PartyQuestSystem;

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
	void SetOwnerSystem(std::shared_ptr<PartyQuestSystem> sys) { m_ownerPartrySystem.swap(sys); }
	std::shared_ptr<PartyQuestSystem> GetOwnerSystem()const noexcept;
public:
	void CheckPartyQuestState()noexcept;
	bool IsClear()const noexcept { return m_isClear.load(); }
protected:
	NagoxAtomic::Atomic<int8_t> m_mon_count{ 0 };
private:
	NagoxAtomic::Atomic<bool> m_isClear{ false };
	NagoxAtomic::Atomic<int8_t> m_numOfMember{ 0 };
	std::shared_ptr<PartyQuestSystem> m_ownerPartrySystem;
	XMap<uint32_t, uint32_t> m_id2idx_table;
};

class FoxQuest
	:public QuestRoom
{
public:
	// TODO: 락 고려
	virtual bool ProcessPartyQuest()noexcept override{
		return 1 == m_mon_count.fetch_sub(1);
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
		return 1 == m_mon_count.fetch_sub(1);
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
	std::atomic_bool m_clear{ false };
	virtual bool ProcessPartyQuest()noexcept override {
		return m_clear.load();
	}
	virtual bool IsFailPartyQuest()const noexcept { return false; }

	//virtual void MigrationAfterBehavior(Field* const prev_field)noexcept override;

	virtual void InitQuestField()noexcept override;
	//virtual void NotifyQuestClear(NagiocpX::ContentsEntity* const entity)const noexcept override;
	//virtual void NotifyQuestFail(NagiocpX::ContentsEntity* const entity)const noexcept override;
};