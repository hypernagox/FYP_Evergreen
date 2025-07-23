#pragma once

#include "pch.h"

class PartyStatusGUI : public udsdx::Component
{
private:
	struct PartyGUI
	{
		std::shared_ptr<udsdx::SceneObject> Panel;
		std::shared_ptr<udsdx::SceneObject> PartyMemberIDText;
	};

public:
	void OnInitialize() override;
	void InitializeContents(const std::vector<uint32_t>& table, const std::vector<std::wstring>& names);
	void AddPartyMember(uint32_t partyMemberID, std::wstring_view partyMemberName);
	void RemovePartyMember(uint32_t partyMemberID);
	void SetPartyLeader(uint32_t partyLeaderID);
	void DisablePartyPanel();
	void OnQuestClear();
	void SetDialogPanelMode(bool isEndDialogue);
	
	void RequestQuestStart();
	void RequestQuestEnd();

private:
	void UpdatePartyPanels();

private:
	std::vector<uint32_t> m_partyMemberIDsCache;
	std::vector<std::wstring> m_partyMemberNamesCache;
	uint32_t m_partyLeaderIndexCache = 0;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_leavePartyButton;

	std::vector<PartyGUI> m_partyPanels;

	std::shared_ptr<udsdx::SceneObject> m_dialogPanel;
	std::shared_ptr<udsdx::SceneObject> m_dialogText;
	std::shared_ptr<udsdx::SceneObject> m_dialogEnterButton;
	std::shared_ptr<udsdx::SceneObject> m_dialogExitButton;
};