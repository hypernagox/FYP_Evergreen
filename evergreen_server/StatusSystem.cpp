#include "pch.h"
#include "StatusSystem.h"
#include "EquipmentSystem.h"

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
	return m_equipSystem->ApplyAtk(GetATK(), victim_origin_hp, victim_hp, victim);
}
