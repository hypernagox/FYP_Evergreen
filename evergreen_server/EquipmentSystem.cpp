#include "pch.h"
#include "EquipmentSystem.h"

int EquipmentSystem::ApplyAtk(
	const int attacker_atk,
	const int victim_origin_hp,
	int& victim_hp, 
	ContentsEntity* const victim) noexcept
{
	int additional_dmg = 0;
	for (const auto equip : m_arrEquip) {
		if (!equip)continue;
		additional_dmg += equip->ApplyAtkEquipment(attacker_atk, victim_origin_hp, victim_hp, victim);
	}
	const int result_dmg = attacker_atk + additional_dmg;
	victim_hp -= result_dmg;
	return result_dmg;
}
