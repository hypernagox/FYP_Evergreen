#pragma once
#include "pch.h"

class HarvestSystem
{

public:
	static void LoadHarvest(
		const std::vector<std::string> key_words,
		const std::wstring_view path = L"")noexcept;
public:
	static const auto& GetHarvestPos()noexcept { return g_harvest_pos; }
private:
	static inline XVector<Vector3> g_harvest_pos = {};
};

