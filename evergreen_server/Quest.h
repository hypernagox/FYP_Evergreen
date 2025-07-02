#pragma once

class NagiocpX::ContentsEntity;

class Quest
{
public:
	Quest(const uint64_t questKey_)noexcept :m_questKey{ questKey_ } {}
	virtual ~Quest()noexcept = default;
public:
	virtual bool OnAchieve(NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity)noexcept = 0;
	virtual void OnReward(NagiocpX::ContentsEntity* const clear_entity)noexcept = 0;
	const auto GetQuestKey()const noexcept { return m_questKey; }
public:
	static Quest* const CreateQuest(const uint64_t quest_id)noexcept;
protected:
	const uint64_t m_questKey;
};

