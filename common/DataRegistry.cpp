#include "pch.h"
#include "DataRegistry.h"
#include "PathManager.h"
#include "json.hpp"
#include "JsonGenerator.h"

namespace Common
{
    inline thread_local std::wstring wstr = {};

    void DataRegistry::Load(const std::wstring_view path) noexcept
    {
        static DataRegistry table;
        g_table = &table;
        std::string errStr;
        int category_start_index = 0;
        int recipe_id = 0;
        std::unordered_map<std::string, nlohmann::ordered_json> weapon_data_map;
        bool weapon_loaded = false;
        if (!JsonGenerator::GenerateJson(path))
        {
            errStr = std::format("Err in json generator");
            exit(0);
        }
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
                    nlohmann::ordered_json jsonData;
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
                            std::vector<ItemCombineInfo> comb;
                            bool flag = true;
                            std::string resultItem;
                            int numOfItem = 0;
                            for (const auto& [attrName, value] : attributes.items())
                            {
                                if (flag && (nlohmann::json::value_t::number_integer == value.type()
                                    || nlohmann::json::value_t::number_unsigned == value.type()))
                                {
                                    comb.emplace_back(attrName, -1, value.get<int>());
                                }
                                else if (!flag || nlohmann::json::value_t::string == value.type())
                                {
                                    if (flag)
                                    {
                                        resultItem = value.get<std::string>();
                                        flag = false;
                                    }
                                    else
                                    {
                                        numOfItem = value.get<int>();
                                    }
                                }
                                else
                                {
                                    throw std::runtime_error{ "Recipe Value Error" };
                                }
                            }
                            if (0 == numOfItem)
                            {
                                throw std::runtime_error{ "Num of Result Item is Zero" };
                            }
                            table.m_mapItemRecipe.emplace(recipe_id, ItemRecipeData{ resultItem,-1, recipe_id,numOfItem, std::move(comb) });
                            table.m_mapRecipeName2Int.emplace(entityName, recipe_id);
                            table.m_mapInt2RecipeName.emplace(recipe_id, entityName);
                            ++recipe_id;
                            continue;
                        }
                        if ("Item" == category)
                        {
                            table.m_dropItemID2String.try_emplace(item_start_index, entityName);
                            table.m_dropItemName2Int.try_emplace(entityName, item_start_index);
                            ++item_start_index;
                        }

                        // TODO: 단순한 스탯이라면 갑옷도 여기서 읽어야할 것 같다.
                        if ("Weapon" == category)
                        {
                            static constinit int g_weapon_id = 0;
                            const auto wid = g_weapon_id++;
                            table.m_mapWeaponID[entityName] = wid;
                            table.m_mapWeaponIDStr[wid] = entityName;
                            if (!weapon_loaded)
                            {
                                const std::wstring weapon_path = RESOURCE_PATH(path) + L"\\json\\Weapon.json";
                                std::ifstream weapon_file{ weapon_path };
                                if (!weapon_file)
                                {
                                    throw std::runtime_error("Failed to open Weapon.json");
                                }

                                nlohmann::ordered_json weapon_json;
                                weapon_file >> weapon_json;

                                for (const auto& [name, data] : weapon_json.items())
                                {
                                    weapon_data_map.emplace(name, data);
                                }

                                weapon_loaded = true;
                            }

                            const auto it = weapon_data_map.find(entityName);
                            if (it != weapon_data_map.end())
                            {
                                const auto& json_obj = it->second;

                                EquipmentStat stat;
                                if (json_obj.contains("atk") && json_obj["atk"].is_number_integer())
                                {
                                    stat.atk = json_obj["atk"].get<int>();
                                }
                                if (json_obj.contains("def") && json_obj["def"].is_number_integer()) 
                                {
                                    stat.def = json_obj["def"].get<int>();
                                }

                                table.m_mapEquipStat[wid] = stat;
                            }
                            else
                            {
                                std::cerr << "[Warning] entityName not found in Weapon.json: " << entityName << "\n";
                            }
                        }

                        if ("Armor" == category)
                        {
                            static constinit int g_armor_id = 0;
                            const auto aid = g_armor_id++;
                            table.m_mapArmorID[entityName] = aid;
                            table.m_mapArmorIDStr[aid] = entityName;
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
                            case nlohmann::json::value_t::object:
                                attributeMap[attrName] = value;
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
        for (auto& [recipe_id, ele] : table.m_mapItemRecipe)
        {
            ele.resultItemID = table.GetItemID(ele.resultItem);
            for (auto& items : ele.itemElements)
            {
                items.itemID = table.GetItemID(items.itemName);
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