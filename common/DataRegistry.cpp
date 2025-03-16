#include "pch.h"
#include "DataRegistry.h"
#include "PathManager.h"
#include "json.hpp"

namespace Common
{
	void DataRegistry::Load(const std::wstring_view path) noexcept
	{
        static DataRegistry table;
        g_table = &table;
        std::string errStr;
        int category_start_index = 0;
        for (const auto& entry : std::filesystem::directory_iterator{ RESOURCE_PATH(path) + L"\\json" })
        {
            if (entry.path().extension() == ".json")
            { 
                try {
                    std::ifstream file{ entry.path() };
                    if (!file)
                    {
                        errStr = std::format("Path Error: {}", entry.path().string());
                        throw std::exception{ errStr.data() };
                    }
                    nlohmann::json jsonData;
                    const std::string category = entry.path().stem().string();
                    const auto category_idx = category_start_index++;
                    table.m_categoryId2str.try_emplace(category_idx, category);
                    table.m_str2category_id.try_emplace(category, category_idx);
                    file >> jsonData;
                    bool flag = false;
                    int entity_start_index = 0;
                    for (const auto& [entityName, attributes] : jsonData.items())
                    {
                        const auto entity_idx = entity_start_index++;
                        table.m_str2detail_id.try_emplace(entityName, entity_idx);
                        table.m_detailType2str[category].try_emplace(entity_idx, entityName);
                        table.m_detail2category_str.try_emplace(entityName, category);
                        std::map<std::string, int> attributeTable;
                        for (const auto& [attrName, value] : attributes.items())
                        {
                            if (!attributeTable.try_emplace(attrName, value.get<int>()).second) 
                            {
                                flag = true;
                                break;
                            }
                        }
                        if (flag || !table.m_mapDatatable[category].try_emplace(entityName, std::move(attributeTable)).second)
                        {
                            errStr = "Name duplicated";
                            throw std::exception{ errStr.data() };
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    std::cout << e.what();
                    exit(1);
                }
            }
        }
	}
}