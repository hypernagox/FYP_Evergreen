#pragma once
#include "ContentsComponent.h"

class EquipmentSystem;
class BuffSystem;
class Skill;

class StatusSystem
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(StatusSystem)
public:
	~StatusSystem()noexcept;
public:
	int GetATK()const noexcept;
	int GetDEF()const noexcept;
public:
	int ApplyAtk(
		const int victim_origin_hp,
		int& victim_hp,
		ContentsEntity* const victim
	);
public:
	template<typename T>
	T* const SetSkill(const Nagox::Enum::SKILL_TYPE skill_type)noexcept {
		return static_cast<T* const>(m_skills[skill_type] = NagiocpX::xnew<T>());
	}
	const bool UseSkill(const Nagox::Enum::SKILL_TYPE skill_type)noexcept;
public:
	int m_defaultATK = 1;
	int m_defaultDEF = 0;
	EquipmentSystem* m_equipSystem = nullptr;
	BuffSystem* m_buffSystem = nullptr;

	Skill* m_skills[Nagox::Enum::SKILL_TYPE::SKILL_TYPE_MAX + 1]{ nullptr };
};

