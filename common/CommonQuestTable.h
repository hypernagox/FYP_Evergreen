#pragma once

namespace Common
{
	// »ç³ÉÄù, ½ÉºÎ¸§Äù µîµî
	enum class CommonQuestType
	{
		KILL_MONSTER,

		END
	};

	struct CommonQuestInfo
	{
		struct QuestMonsterInfo {
			std::wstring mon_name;
			int mon_count;
		};
		struct QuestRewardInfo {
			std::wstring item_name;
			int item_id;
			int item_count;
		};
		int quest_id;
		CommonQuestType quest_type;
		std::wstring quest_name;
		std::wstring quest_giver;
		std::wstring quest_destination;
		std::vector<QuestMonsterInfo> monsters_info;
		std::vector<QuestRewardInfo> reward_info;
		int reward_gold;
	};

	class CommonQuestTable
	{
	public:
		static void LoadCommonQuest(const std::wstring_view json_path_w = {})noexcept;
	public:

		static inline  const CommonQuestInfo& GetCommonQuestInfo(const uint64_t quest_id)noexcept{
			return m_id2QuestInfo[quest_id];
		}
		static inline  const CommonQuestInfo& GetCommonQuestInfo(const std::wstring_view quest_name)noexcept {
			return m_name2QuestInfo[quest_name.data()];
		}
	private:
		static inline std::map<uint64_t, CommonQuestInfo> m_id2QuestInfo;
		static inline std::map<std::wstring, CommonQuestInfo> m_name2QuestInfo;
	};
}
