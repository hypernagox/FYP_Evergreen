#pragma once

class ServerCore::ContentsEntity;

class Quest
{
public:
	Quest()noexcept = default;
	virtual ~Quest()noexcept = default;
public:
	virtual bool OnAchieve(ServerCore::ContentsEntity* const key_entity, ServerCore::ContentsEntity* const clear_entity)noexcept = 0;
	virtual void OnReward(ServerCore::ContentsEntity* const clear_entity)noexcept = 0;
private:
	
};

class KillFoxQuest
	:public Quest
{
public:
	virtual bool OnAchieve(ServerCore::ContentsEntity* const key_entity, ServerCore::ContentsEntity* const clear_entity)noexcept override;
	virtual void OnReward(ServerCore::ContentsEntity* const clear_entity)noexcept override;
private:
	uint32_t m_curCount = 0;
	const uint32_t m_clearCount = 5;
};