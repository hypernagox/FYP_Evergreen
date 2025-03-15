#pragma once
#include "ContentsComponent.h"

class HP
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(HP)
public:
	void SetHP(const int hp_val)noexcept { m_hp = hp_val; }
	void SetMaxHP(const int max_hp)noexcept { m_maxHP = max_hp; }
	void InitHP(const int max_hp) {
		SetMaxHP(max_hp);
		SetHP(max_hp);
	}
	void CompleteRebirth(const int hp_) {
		InitHP(hp_);
		m_bIsRebirth = false;
	}
public:
	int GetCurHP()const { return m_hp; }
	int GetMaxHP()const { return m_maxHP; }
public:
	void PostDoDmg(const int dmg_, NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject)noexcept;
	void PostDoHeal(const int heal_)noexcept;
public:
	void ProcessCleanUp()noexcept override {
		SetHP(m_maxHP);
		m_bIsRebirth = false;
	}
private:
	void DoDmg(const int dmg_, const NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject)noexcept;
	void DoHeal(const int heal_)noexcept;
private:
	int m_hp = -1;
	int m_maxHP = -1;
	bool m_bIsRebirth = false;
};

