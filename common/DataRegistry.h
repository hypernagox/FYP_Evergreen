#pragma once

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
		int GetObjectData(
			const std::string_view obj_name,
			const std::string_view att_name
		)const noexcept {
			const auto iter = m_detail2category_str.find(obj_name.data());
			return m_mapDatatable.find(iter->second)->second.find(obj_name.data())->second.find(att_name.data())->second;
		}
	private:
		using DataTable = std::map<std::string, std::map<std::string, int>>;
		std::map<std::string, DataTable> m_mapDatatable;

		// ������Ʈ�� ��з� -> ��Ʈ�� ��) 1 -> Monster
		std::map<int, std::string> m_categoryId2str;
		std::map<std::string, int> m_str2category_id;
		// Fox�� ���� ī�װ��ΰ�?
		std::map<std::string, std::string> m_detail2category_str;

		// �󼼺з� -> ��) 1 -> Fox
		// ������ ���� appear������Ʈ�� ī�װ��� 1, �����ϰ� 1 ���� �̰� Monster->Fox�� �ǹ�
		std::map<std::string, std::map<int, std::string>> m_detailType2str;
		std::map<std::string, int> m_str2detail_id;


		constinit static inline const DataRegistry* g_table = nullptr;
	};

#define DATA_TABLE (Common::DataRegistry::GetDataTable())
}
