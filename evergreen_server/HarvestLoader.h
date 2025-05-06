#pragma once
#include "pch.h"

struct HarvestInfo
{
	Vector3 harvest_pos;
	HARVEST_TYPE harvest_type;
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
private:
	constinit static inline XVector<HarvestInfo>* g_harvest_pos = nullptr;
};

