#pragma once

class Equipment
{
	friend class EquipmentSystem;
	friend class ObjectIdentifier;
public:
	int GetATK()const noexcept { return atk; }
	int GetDEF()const noexcept { return def; }

	virtual int ApplyAtkEquipment(
		ContentsEntity* const atk_obj,
		const int attacker_atk,
		const int victim_origin_hp,
		int& victim_hp,
		ContentsEntity* const victim)noexcept 
	{
		return 0;
	}
private:
	int id = 1;
	int atk = 1;
	int def = 0;
};

class EquipmentSystem
{
public:
	EquipmentSystem()noexcept;
	~EquipmentSystem()noexcept;
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
		ContentsEntity* const atk_obj,
		const int attacker_atk,
		const int victim_origin_hp,
		int& victim_hp,
		ContentsEntity* const victim)noexcept;
public:
	const auto GetEquipment(const Nagox::Enum::EQUIPMENT_TYPE equip_type)noexcept {
		return m_arrEquip[equip_type];
	}
public:
	bool SwapEquipment(
		class ContentsEntity* const owner,
		const Nagox::Enum::EQUIPMENT_TYPE equip_type,
		const uint32_t equip_id)noexcept;
private:
	Equipment* m_arrEquip[(int)Nagox::Enum::EQUIPMENT_TYPE_MAX + 1]{ nullptr };
};

