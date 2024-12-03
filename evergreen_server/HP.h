#pragma once
#include "ContentsComponent.h"

class HP
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(HP)
public:
	void InitHP(const int hp_) { m_hp = hp_; }
	void CompleteRebirth(const int hp_) {
		InitHP(hp_);
		m_bIsRebirth = false;
	}
public:
	void PostDoDmg(const int dmg_, ServerCore::S_ptr<ServerCore::ContentsEntity> atkObject)noexcept;
	void PostDoHeal(const int heal_)noexcept;
private:
	void DoDmg(const int dmg_, const ServerCore::S_ptr<ServerCore::ContentsEntity> atkObject)noexcept;
	void DoHeal(const int heal_)noexcept;
private:
	int m_hp = 3;
	bool m_bIsRebirth = false;
};

