#include "pch.h"
#include "StatusSystem.h"
#include "EquipmentSystem.h"
#include "Skill.h"

StatusSystem::~StatusSystem() noexcept
{
	for (const auto skill : m_skills)
	{
		if (!skill)continue;
		NagiocpX::xdelete<Skill>(skill);
	}
}

// TODO: 버프시스템고려
int StatusSystem::GetATK() const noexcept
{
	return m_defaultATK + m_equipSystem->GetEquipmentAtkSum();
}

int StatusSystem::GetDEF() const noexcept
{
	return 0;
}

int StatusSystem::ApplyAtk(const int victim_origin_hp, int& victim_hp, ContentsEntity* const victim)
{
	return m_equipSystem->ApplyAtk(GetOwnerEntityRaw(), GetATK(), victim_origin_hp, victim_hp, victim);
}

const bool StatusSystem::UseSkill(const Nagox::Enum::SKILL_TYPE skill_type) noexcept
{
	if (m_skills[skill_type])return m_skills[skill_type]->UseSkill(this);
	else return false;
}
