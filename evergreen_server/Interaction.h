#pragma once
#include "ContentsComponent.h"
#include "Navigator.h"

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
	void SetNavMeshType(const NAVI_MESH_TYPE nav_mesh_type) { m_nav_mesh_type = nav_mesh_type; }
private:
	std::mutex m_clear_tree_mutex;
	int8_t m_num_of_reward_count = 5;
	NAVI_MESH_TYPE m_nav_mesh_type = NAVI_MESH_TYPE::MAIN_WORLD;
	uint64_t m_last_get_time = 0;
};

class Catapult
	:public Interaction
{
public:
	Catapult(const auto pOwner) :Interaction{ pOwner } {}
public:
	virtual bool DoInteraction(ContentsEntity* const pEntity_)noexcept override;
public:
	S_ptr<ContentsEntity> m_boss_ptr = {};
	class FireMeteor* m_meteor_node = nullptr;
};

