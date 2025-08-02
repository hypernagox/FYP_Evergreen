#pragma once

class Skill
{
public:
	virtual ~Skill()noexcept = default;
public:
	void SetCoolDown(const uint64_t cool_down)noexcept{
		m_cool_down = cool_down;
	}
public:
	const bool IsCoolDown()const noexcept {
		return IsCoolDown(::GetTickCount64());
	}
	const bool UseSkill(class StatusSystem* const use_entity_system)noexcept {
		const auto cur_time = ::GetTickCount64();
		if (!IsCoolDown(cur_time))return false;
		const bool res = ExecuteSkill(use_entity_system);
		if (res)m_last_used_time = cur_time;
		return res;
	}
protected:
	const bool IsCoolDown(const uint64_t cur_time)const noexcept {
		const auto diff = cur_time - m_last_used_time;
		const bool res = diff >= m_cool_down;
		if (!res) {
			//std::cout << "남은 쿨타임: " << (float)(m_cool_down - diff) / 1000.f << "초\n";
		}
		return res;
		//return cur_time - m_last_used_time >= m_cool_down;
	}
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept = 0;
private:
	uint64_t m_cool_down = 250;
	uint64_t m_last_used_time = 0;
};

class WarriorDefaultAttack
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};

class WarriorSkill_1
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};

class PriestDefaultAttack
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};

class PriestSkill_1
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};

class ArcherDefaultAttack
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};

class ArcherSkill_1
	:public Skill
{
	virtual bool ExecuteSkill(class StatusSystem* const use_entity_system)noexcept override;
};