#pragma once

enum class EQUIPMENT_PART:uint8_t
{
	WEAPON = 0,



	END,
};

class Equipment
{
public:
	int GetATK()const noexcept { return atk; }
	int GetDEF()const noexcept { return def; }

	virtual int ApplyAtkEquipment(
		const int attacker_atk,
		const int victim_origin_hp,
		int& victim_hp,
		ContentsEntity* const victim)noexcept 
	{
		return 0;
	}
//private:
	int id;
	int atk = 0;
	int def = 0;
};

class EquipmentSystem
{
public:
	int GetEquipmentAtkSum()const noexcept {
		int sum = 0;
		for (const auto equip : m_arrEquip) {
			if (!equip)continue;
			sum += equip->GetATK();
		}
		return sum;
	}
	int ApplyAtk(
		const int attacker_atk,
		const int victim_origin_hp,
		int& victim_hp,
		ContentsEntity* const victim)noexcept;
private:
	Equipment* m_arrEquip[(int)EQUIPMENT_PART::END]{ nullptr };
};

