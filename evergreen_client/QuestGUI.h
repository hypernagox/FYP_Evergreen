#pragma once

#include "pch.h"

class QuestGUI : public udsdx::Component
{
private:
	struct QuestContent
	{
		udsdx::GUIButton* ButtonPanel;
		udsdx::GUIImage* IconImage;
		udsdx::GUIText* NameText;
		udsdx::GUIText* DescriptionText;
	};

	struct PartyContent
	{
		udsdx::GUIButton* ButtonPanel;
		udsdx::GUIImage* IconImage;
		udsdx::GUIText* NameText;
		udsdx::GUIText* DescriptionText;
	};

public:
	void OnInitialize() override;

	void RequestPartyList(int questID);
	void FetchQuestList();
	void FetchPartyList(const std::vector<uint32_t>& table);
	void UpdateQuestPage(int page);
	void UpdatePartyPage(int page);
	int GetNumQuestPages() const { return (m_numQuests + SIZE_QUEST_LIST - 1) / SIZE_QUEST_LIST; }
	int GetNumPartyPages() const { return (m_numParties + SIZE_PARTY_LIST - 1) / SIZE_PARTY_LIST; }
	std::shared_ptr<udsdx::SceneObject> GetQuestListPanel() const { return m_questListPanel; }
	std::shared_ptr<udsdx::SceneObject> GetPartyListPanel() const { return m_partyListPanel; }

private:
	static constexpr int SIZE_QUEST_LIST = 5;
	static constexpr int SIZE_PARTY_LIST = 4;

	int m_selectedQuestID = 0;
	int m_currentQuestPage = 0;
	int m_currentPartyPage = 0;

	int m_numQuests = 0;
	int m_numParties = 0;

	std::shared_ptr<udsdx::SceneObject> m_standByText;
	std::shared_ptr<udsdx::SceneObject> m_questListPanel;
	std::shared_ptr<udsdx::SceneObject> m_partyListPanel;
	std::shared_ptr<udsdx::SceneObject> m_partyCreatePanel;

	std::array<QuestContent, SIZE_QUEST_LIST> m_questList;
	std::array<PartyContent, SIZE_PARTY_LIST> m_partyList;

	udsdx::GUIText* m_selectedQuestText;

	udsdx::GUIButton* m_incrementQuestButton;
	udsdx::GUIButton* m_decrementQuestButton;
	udsdx::GUIButton* m_incrementPartyButton;
	udsdx::GUIButton* m_decrementPartyButton;

	std::vector<uint32_t> m_partyTableCache;
};

class PartyQuestTable
{
public:
	static void InitPartyQuestTable(const std::wstring_view path = L"") noexcept;
public:
	static const std::wstring& GetPartyQuestStr(const int party_quest_id) noexcept { return m_id_to_name[party_quest_id]; }
	static const int GetPartyQuestInt(const std::wstring_view party_quest_name) noexcept { return m_name_to_id[party_quest_name.data()]; }
	static const std::wstring& GetPartyQuestDescription(const int party_quest_id) noexcept { return m_id_to_description[party_quest_id]; }
	static const std::wstring& GetPartyQuestIcon(const int party_quest_id) noexcept { return m_id_to_icon[party_quest_id]; }
	static const size_t GetPartyQuestSize() noexcept { return m_id_to_name.size(); }
private:
	static inline std::map<int, std::wstring> m_id_to_name;
	static inline std::map<std::wstring, int> m_name_to_id;
	static inline std::map<int, std::wstring> m_id_to_description;
	static inline std::map<int, std::wstring> m_id_to_icon;
};