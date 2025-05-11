#pragma once
#include "pch.h"

struct HarvestInfo
{
	Vector3 harvest_pos;
	HARVEST_TYPE harvest_type;
	uint16_t harvest_mesh_type = 1;
};

class HarvestLoader
{
public:
	static void LoadHarvest(
		const std::vector<std::string> key_words,
		const std::wstring_view path = L"")noexcept;
	static void FreeHarvestLoader()noexcept;
public:
	static const auto& GetHarvestPos()noexcept { return *g_harvest_pos; }
	static const auto GetHarvestMaxType()noexcept { return g_harvest_type_number; }
private:
	constinit static inline XVector<HarvestInfo>* g_harvest_pos = nullptr;
	static constinit inline uint16_t g_harvest_type_number = 1;
};

