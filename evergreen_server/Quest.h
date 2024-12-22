#pragma once

class ServerCore::ContentsEntity;

class Quest
{
public:
	Quest(const uint64_t questKey_)noexcept :m_questKey{ questKey_ } {}
	virtual ~Quest()noexcept = default;
public:
	virtual bool OnAchieve(ServerCore::ContentsEntity* const key_entity, ServerCore::ContentsEntity* const clear_entity)noexcept = 0;
	virtual void OnReward(ServerCore::ContentsEntity* const clear_entity)noexcept = 0;
	const auto GetQuestKey()const noexcept { return m_questKey; }
private:
	const uint64_t m_questKey;
};

class KillFoxQuest
	:public Quest
{
public:
	KillFoxQuest(const uint64_t questKey_)noexcept :Quest{ questKey_ } {}
public:
	virtual bool OnAchieve(ServerCore::ContentsEntity* const key_entity, ServerCore::ContentsEntity* const clear_entity)noexcept override;
	virtual void OnReward(ServerCore::ContentsEntity* const clear_entity)noexcept override;
private:
	uint32_t m_curCount = 0;
	const uint32_t m_clearCount = 5;
};