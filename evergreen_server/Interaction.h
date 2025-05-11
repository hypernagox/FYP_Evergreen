#pragma once
#include "ContentsComponent.h"

class ContentsEntity;


// 0이 아니면 하베스트로 판정
class Interaction
	:public ContentsComponent
{
	CONSTRUCTOR_CONTENTS_COMPONENT(Interaction)
public:
	virtual bool DoInteraction(ContentsEntity* const pEntity_)noexcept = 0;
	const uint16_t GetInteractionType()const noexcept { return m_interaction_type; }
	void SetInteractionType(const uint16_t type)noexcept { m_interaction_type = type; }
protected:
	uint16_t m_interaction_type = -1;
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

class ClearTreeInteraction
	:public Interaction
{
public:
	ClearTreeInteraction(const auto pOwner) :Interaction{ pOwner } {}
public:
	virtual bool DoInteraction(ContentsEntity* const pEntity_)noexcept override;
private:
	std::mutex m_clear_tree_mutex;
	int8_t m_num_of_reward_count = 5;
};

