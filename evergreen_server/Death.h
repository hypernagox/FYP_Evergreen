#pragma once
#include "ContentsComponent.h"

class Death
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(Death)
public:
	virtual void ProcessDeath()noexcept = 0;
};

class PlayerDeath
	:public Death
{
public:
	PlayerDeath(const auto pOwner) :Death{ pOwner } {}


	void ProcessDeath()noexcept override;
private:

};

class MonsterDeath
	:public Death
{
public:
	MonsterDeath(const auto pOwner) :Death{ pOwner } {}

	void ProcessDeath()noexcept override;
private:

};
