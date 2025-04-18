#pragma once
#include "Field.h"

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
public:
	virtual void InitQuestField()noexcept;
	virtual void MigrationAfterBehavior(Field* const prev_field)noexcept override;
	void DecMemberCount()noexcept;
	void IncMemberCount()noexcept { m_numOfMember.fetch_add(1); }
	const auto GetMemberCount()const noexcept { return m_numOfMember.load(); }
	void SetOwnerSystem(class PartyQuestSystem* sys) { m_ownerPartrySystem = sys; }
	const auto GetOwnerSystem()const noexcept { return m_ownerPartrySystem; }

	int tempid = 0;
private:
	NagoxAtomic::Atomic<int8_t> m_numOfMember{ 0 };
	class PartyQuestSystem* m_ownerPartrySystem = nullptr;
};

