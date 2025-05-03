#pragma once
#include "ContentsComponent.h"

class ContentsEntity;

class Interaction
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(Interaction)
public:
	virtual bool DoInteraction(ContentsEntity* const pEntity_)noexcept = 0;
private:

};

class HarvestInteraction
	:public Interaction
{
public:
	HarvestInteraction(const auto pOwner) :Interaction{ pOwner } {}
public:
	virtual bool DoInteraction(ContentsEntity* const pEntity_)noexcept override;
private:
	NagoxAtomic::Atomic<bool> m_isActive{ true };

	static constexpr uint64_t g_harvest_cool_down = 10000;
};

