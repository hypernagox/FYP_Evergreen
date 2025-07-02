#include "pch.h"
#include "CommonQuestTable.h"
#include "DataRegistry.h"
#include "PathManager.h"

namespace Common
{

	void CommonQuestTable::LoadCommonQuest(const std::wstring_view json_path_w) noexcept
	{
		const auto path = RESOURCE_PATH(json_path_w) + L"\\quest_data\\quest_data.json";

		std::ifstream ifs{ path.data() };

		if (!ifs)
			return;

		nlohmann::json root;
		ifs >> root;

		for (const auto& q : root)
		{
			const std::string type_str = q.at("type").get<std::string>();

			CommonQuestInfo info;
			info.quest_id = q.at("id").get<int>();
			auto str = Common::DataRegistry::Str2Wstr(q.at("name").get<std::string>());;
			info.quest_name = std::move(str);
			
			info.quest_giver = Common::DataRegistry::Str2Wstr(q.at("giver").get<std::string>());
			info.quest_destination = Common::DataRegistry::Str2Wstr(q.at("destination").get<std::string>());
			info.reward_gold = q.at("gold").get<int>();
			for (const auto& [mon, count] : q.at("monsters").items())
			{
				auto str = Common::DataRegistry::Str2Wstr(mon);
				info.monsters_info.emplace_back(std::move(str), count.get<int>());
			}
			for (const auto& [item, count] : q.at("rewards").items())
			{
				auto str = Common::DataRegistry::Str2Wstr(item);
				info.reward_info.emplace_back(std::move(str), count.get<int>());
			}

			m_id2QuestInfo[info.quest_id] = info;
			m_name2QuestInfo[info.quest_name] = info;

			if (type_str == "KillMonster")
			{
				info.quest_type = CommonQuestType::KILL_MONSTER;
			}
			// 다른 타입 퀘 추가
		}
	
	}
}