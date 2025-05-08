#pragma once

// ������ Ŭ�� ���ʿ��� ���� �ؾ� �� ���� �� enum��

static constexpr const uint32_t MAX_QUICK_SLOT = 3;
static constexpr const uint8_t NUM_OF_MAX_QUESTS = 10;
static constexpr const uint8_t NUM_OF_MAX_INVENTORY_ITEM = 30;

static constexpr const int8_t NUM_OF_MAX_PARTY_MEMBER = 3;
static constexpr const int8_t NUM_OF_PARTYQUEST = 3;

constexpr const float HARVEST_INTERACTION_DIST = 5.f;

constexpr const float TERRAIN_OFFSET = 512.f;

constexpr const uint16_t DISTANCE_FILTER = 64;


enum class HARVEST_TYPE: int8_t
{
	LILLY = 0,
	BUSH = 1,
	ROCK = 2,

};

enum class HARVEST_STATE :int8_t
{
	UNAVAILABLE = 0,
	AVAILABLE = 1
};