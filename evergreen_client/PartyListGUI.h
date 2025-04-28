#pragma once

#include "pch.h"

class PartyListGUI : public udsdx::Component
{
private:
	struct PartyGUI
	{
		std::shared_ptr<udsdx::SceneObject> Panel;
		std::shared_ptr<udsdx::SceneObject> PartyMemberIDText;
		std::shared_ptr<udsdx::SceneObject> JoinButton;
	};

public:
	PartyListGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void UpdateContents(const std::vector<uint32_t>& table);
	void UpdateQuestID(int questID);

private:
	// TODO: Replace with actual number of quests
	static constexpr int NumQuests = 3;

	int m_currentQuestID = 0;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_selectedQuestText;
	std::shared_ptr<udsdx::SceneObject> m_incrementQuestButton;
	std::shared_ptr<udsdx::SceneObject> m_decrementQuestButton;
	std::shared_ptr<udsdx::SceneObject> m_refreshQuestButton;
	std::shared_ptr<udsdx::SceneObject> m_standByText;
	std::shared_ptr<udsdx::SceneObject> m_createPartyButton;
	std::shared_ptr<udsdx::SceneObject> m_createPartyText;
	std::vector<PartyGUI> m_partyPanels;
};

