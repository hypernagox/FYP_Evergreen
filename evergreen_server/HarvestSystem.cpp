#include "pch.h"
#include "HarvestSystem.h"
#include "DataRegistry.h"

void HarvestSystem::LoadHarvest(const std::vector<std::string> key_words, const std::wstring_view path) noexcept
{
	std::ifstream file{ RESOURCE_PATH(path) };
	nlohmann::json j;
	file >> j;
	//const auto terrain_scale = GET_DATA(float, "TerrainSize", "Value");
	const auto terrain_scale = 1.f;
	for (const auto& prototype : j)
	{
		const std::string key_name = prototype["prefab"];
		bool flag = key_words.empty();
		for (const auto& key_word : key_words)
		{
			if (key_name.find(key_word) != std::string::npos)
			{
				flag = true;
				break;
			}
		}
		if (!flag)continue;
		for (const auto& instance : prototype["instances"])
		{
			const Vector3 position = Vector3(instance["position"]["x"], instance["position"]["y"], instance["position"]["z"]);
			g_harvest_pos.emplace_back(position * terrain_scale);
		}
	}
}
