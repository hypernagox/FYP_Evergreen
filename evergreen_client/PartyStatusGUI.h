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
	PartyStatusGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void InitializeContents(const std::vector<uint32_t>& table);
	void AddPartyMember(uint32_t partyMemberID);
	void RemovePartyMember(uint32_t partyMemberID);
	void DisablePartyPanel();

private:
	void UpdatePartyPanels();

private:
	std::vector<uint32_t> m_partyMemberIDsCache;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_titleText;
	std::shared_ptr<udsdx::SceneObject> m_leavePartyButton;

	std::vector<PartyGUI> m_partyPanels;
};

