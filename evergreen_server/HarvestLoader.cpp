#include "pch.h"
#include "HarvestLoader.h"
#include "DataRegistry.h"

void HarvestLoader::LoadHarvest(const std::vector<std::string> key_words, const std::wstring_view path) noexcept
{
	std::ifstream file{ RESOURCE_PATH(path) };
	nlohmann::json j;
	file >> j;
	//const auto terrain_scale = GET_DATA(float, "TerrainSize", "Value");
	g_harvest_pos = NagiocpX::xnew<XVector<HarvestInfo>>();
	const auto terrain_scale = 1.f;
	// 반드시 1로시작
	uint16_t harvest_mesh_type = 1;
	for (const auto& prototype : j)
	{
		HARVEST_TYPE type = (HARVEST_TYPE)0;
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
		if (key_name.find("Lilly") != std::string::npos)
		{
			type = HARVEST_TYPE::LILLY;
		}
		else if (key_name.find("Bush") != std::string::npos)
		{
			type = HARVEST_TYPE::BUSH;
		}
		else
		{
			type = HARVEST_TYPE::ROCK;
		}
		for (const auto& instance : prototype["instances"])
		{
			const Vector3 position = Vector3(instance["position"]["x"], instance["position"]["y"], instance["position"]["z"]);
			g_harvest_pos->emplace_back(position * terrain_scale, type, harvest_mesh_type++);
		}
	}
}

void HarvestLoader::FreeHarvestLoader() noexcept
{
	if (!g_harvest_pos)return;
	NagiocpX::xdelete<XVector<HarvestInfo>>(g_harvest_pos);
	g_harvest_pos = nullptr;
}
