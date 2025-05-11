#pragma once
#include "ContentsComponent.h"

class EquipmentSystem;
class BuffSystem;

class StatusSystem
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(StatusSystem)
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
	int m_defaultATK = 1;
	int m_defaultDEF = 1;
	EquipmentSystem* m_equipSystem = nullptr;
	BuffSystem* m_buffSystem = nullptr;
};

