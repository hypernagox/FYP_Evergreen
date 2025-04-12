#pragma once
#include "pch.h"

struct ItemCombineInfo {
	std::string itemName;
	int itemID;
	int numOfRequire;
};

struct ItemRecipeData {
	std::string resultItem;
	int resultItemID;
	int recipeID;
	int numOfResultItem;
	std::vector<ItemCombineInfo> itemElements;
};

namespace Common
{
	class DataRegistry
	{
	private:
		DataRegistry()noexcept = default;
	public:
		static void Load(const std::wstring_view path = L"")noexcept;
		static const auto GetDataTable()noexcept { return g_table; }
	public:
		const int GetItemID(const std::string_view drop_item_name)const noexcept {
			// TODO: 없는 아이템이 원래 있으면 안됨
			const auto iter = m_dropItemName2Int.find(drop_item_name.data());
			if (m_dropItemName2Int.end() != iter)
				return iter->second;
			else
				return -1;
			return m_dropItemName2Int.find(drop_item_name.data())->second;
		}
		const std::string& GetItemName(const int drop_item_id)const noexcept {
			// TODO: 없는 아이템이 원래 있으면 안됨
			const auto iter = m_dropItemID2String.find(drop_item_id);
			if (m_dropItemID2String.end() != iter)
				return iter->second;
			else
				return {};
			return m_dropItemID2String.find(drop_item_id)->second;
		}
	public:
		const std::string& GetId2Category(const int id)const {
			return m_categoryId2str.find(id)->second;
		}
		const std::string& GetId2Detail(const std::string_view category_str, const int id)const {
			return m_detailType2str.find(category_str.data())->second.find(id)->second;
		}
		int GetStr2Category(const std::string_view str)const {
			return m_str2category_id.find(str.data())->second;
		}
		int GetStr2Detail(const std::string_view str)const {
			return m_str2detail_id.find(str.data())->second;
		}
	public:
		static const std::wstring& Str2Wstr(const std::string_view str) noexcept;
	public:
		const int GetRecipeIntID(const std::string_view recipe_name)const noexcept {
			return m_mapRecipeName2Int.find(recipe_name.data())->second;
		}
		const std::string GetRecipeName(const int recipe_id)const noexcept {
			return m_mapInt2RecipeName.find(recipe_id)->second;
		}
		const auto& GetItemRecipe(const std::string_view recipe_name)const noexcept {
			// TODO: 없으면걍 폭
			return (const_cast<DataRegistry*>(this))->m_mapItemRecipe[GetRecipeIntID(recipe_name)];
			return m_mapItemRecipe.find(GetRecipeIntID(recipe_name))->second;
		}
		const auto& GetItemRecipe(const int recipe_id)const noexcept {
			return (const_cast<DataRegistry*>(this))->m_mapItemRecipe[recipe_id];
			return m_mapItemRecipe.find(recipe_id)->second;
		}
		template<typename T>
		const T& GetObjectData(const std::string_view obj_name, const std::string_view att_name) const {
#ifdef _DEBUG
#define DBG_ASSERT(cond, msg) \
        if (!(cond)) { \
            assert(false && msg); \
        }
#else
#define DBG_ASSERT(cond, msg) ((void)0)
#endif
			const auto categoryIter = m_detail2category_str.find(obj_name.data());
			DBG_ASSERT(categoryIter != m_detail2category_str.end(), "Object not found");
			const auto& category = categoryIter->second;
			const auto objIter = m_mapDatatable.find(category);
			DBG_ASSERT(objIter != m_mapDatatable.end(), "Category not found");
			const auto& dataTable = objIter->second;
			const auto attrMapIter = dataTable.find(obj_name.data());
			DBG_ASSERT(attrMapIter != dataTable.end(), "Object attributes not found");
			const auto& attrMap = attrMapIter->second;
			const auto attrIter = attrMap.find(att_name.data());
			DBG_ASSERT(attrIter != attrMap.end(), "Attribute not found");
			if constexpr (std::same_as<T, std::wstring>) {
				DBG_ASSERT(std::holds_alternative<std::string>(attrIter->second), "Type mismatch for attribute");
				return Str2Wstr(std::get<std::string>(attrIter->second));
			}
			else {
				DBG_ASSERT(std::holds_alternative<T>(attrIter->second), "Type mismatch for attribute");
				return std::get<T>(attrIter->second);
			}
		}
	private:
		using AttributeValue = std::variant<int, bool, float, std::string>;
		using AttributeMap = std::map<std::string, AttributeValue>;
		using DataTable = std::map<std::string, AttributeMap>;
		std::map<std::string, DataTable> m_mapDatatable;

		// 오브젝트의 대분류 -> 스트링 예) 1 -> Monster
		std::map<int, std::string> m_categoryId2str;
		std::map<std::string, int> m_str2category_id;
		// Fox는 무슨 카테고리인가?
		std::map<std::string, std::string> m_detail2category_str;

		// 상세분류 -> 예) 1 -> Fox
		// 서버로 부터 appear오브젝트로 카테고리값 1, 디테일값 1 오면 이건 Monster->Fox를 의미
		std::map<std::string, std::map<int, std::string>> m_detailType2str;
		std::map<std::string, int> m_str2detail_id;


		std::map<std::string, int> m_dropItemName2Int;
		std::map<int, std::string> m_dropItemID2String;

		constinit static inline const DataRegistry* g_table = nullptr;


		// ----------------- 아이템 레시피 테이블 --------------------
		std::map<int, ItemRecipeData> m_mapItemRecipe;
		std::map<std::string, int> m_mapRecipeName2Int;
		std::map<int, std::string> m_mapInt2RecipeName;
		// ---------------------------------------------------------
	};

#define DATA_TABLE (Common::DataRegistry::GetDataTable())
#define GET_DATA(type, obj_name, attr_name) (DATA_TABLE->GetObjectData<type>(obj_name, attr_name))
#define GET_RECIPE(item_name) (DATA_TABLE->GetItemRecipe(item_name))
}
