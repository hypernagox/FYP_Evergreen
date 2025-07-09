#pragma once

#include "pch.h"

class PartyStatusGUI : public udsdx::Component
{
private:
	struct PartyGUI
	{
		std::shared_ptr<udsdx::SceneObject> Panel;
		std::shared_ptr<udsdx::SceneObject> PartyMemberIDText;
		std::shared_ptr<udsdx::SceneObject> PartyLeaderIcon;
	};

public:
	void OnInitialize() override;
	void InitializeContents(const std::vector<uint32_t>& table);
	void AddPartyMember(uint32_t partyMemberID);
	void RemovePartyMember(uint32_t partyMemberID);
	void SetPartyLeader(uint32_t partyLeaderID);
	void DisablePartyPanel();

private:
	void UpdatePartyPanels();

private:
	std::vector<uint32_t> m_partyMemberIDsCache;
	uint32_t m_partyLeaderIndexCache = 0;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_titleText;
	std::shared_ptr<udsdx::SceneObject> m_leavePartyButton;

	std::vector<PartyGUI> m_partyPanels;
};

