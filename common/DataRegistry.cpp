#include "pch.h"
#include "DataRegistry.h"
#include "PathManager.h"
#include "json.hpp"

namespace Common
{
    inline thread_local std::wstring wstr = {};

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
                    int entity_start_index = 0;
                    int item_start_index = 0;
                    for (const auto& [entityName, attributes] : jsonData.items())
                    {
                        // TODO: 이렇게 해야하는 오브젝트 종류가 늘어나면 쌉 하드코딩 각이 보인다.
                        if ("ItemRecipe" == category)
                        {
                            for (const auto& [attrName, value] : attributes.items())
                            {
                                if (nlohmann::json::value_t::number_integer == value.type()
                                    || nlohmann::json::value_t::number_unsigned == value.type())
                                {
                                    table.m_mapItemRecipe[entityName].emplace_back(attrName, value.get<int>());
                                }
                                else
                                {
                                    throw std::runtime_error{ "Recipe Value Error" };
                                }
                            }
                            continue;
                        }
                        if ("Item" == category)
                        {
                            table.m_dropItemID2String.try_emplace(item_start_index, entityName);
                            table.m_dropItemName2Int.try_emplace(entityName, item_start_index);
                            ++item_start_index;
                        }

                        const auto entity_idx = entity_start_index++;
                        table.m_str2detail_id.try_emplace(entityName, entity_idx);
                        table.m_detailType2str[category].try_emplace(entity_idx, entityName);
                        table.m_detail2category_str.try_emplace(entityName, category);

                        AttributeMap attributeMap;

                        for (const auto& [attrName, value] : attributes.items())
                        {
                            if (attributeMap.contains(attrName)) 
                            {
                                throw std::runtime_error("Duplicate attribute: " + std::string(attrName));
                            }

                            switch (value.type())
                            {
                            case nlohmann::json::value_t::number_integer:
                                attributeMap[attrName] = value.get<int>();
                                break;
                            case nlohmann::json::value_t::number_unsigned:
                                attributeMap[attrName] = static_cast<int>(value.get<unsigned int>());
                                break;
                            case nlohmann::json::value_t::number_float:
                                attributeMap[attrName] = value.get<float>();
                                break;
                            case nlohmann::json::value_t::boolean:
                                attributeMap[attrName] = value.get<bool>();
                                break;
                            case nlohmann::json::value_t::string:
                                attributeMap[attrName] = value.get<std::string>();
                                break;
                            default:
                                throw std::runtime_error("Unsupported attribute type for " + std::string(attrName));
                            }
                        }

                        auto& categoryMap = table.m_mapDatatable[category];

                        if (categoryMap.contains(entityName)) 
                        {
                            throw std::runtime_error("Duplicate entity: " + std::string(entityName));
                        }

                        categoryMap[entityName] = std::move(attributeMap);
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
    const std::wstring& DataRegistry::Str2Wstr(const std::string_view str) noexcept
    {
        extern thread_local std::wstring wstr;
        const int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0);
        wstr.clear();
        wstr.resize(size_needed);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, &wstr[0], size_needed);
        wstr.pop_back();
        return wstr;
    }
}