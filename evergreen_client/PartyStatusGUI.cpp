#include "pch.h"
#include "PartyStatusGUI.h"

using namespace udsdx;

PartyStatusGUI::PartyStatusGUI(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(540.0f, 360.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2(480.0f, 320.0f));

	object->AddChild(m_panel);

	m_titleText = std::make_shared<SceneObject>();
	auto titleText = m_titleText->AddComponent<GUIText>();
	titleText->SetFont(font);
	m_titleText->GetTransform()->SetLocalPosition(Vector3(-230.0f, 140.0f, 0.0f));
	titleText->SetRaycastTarget(false);
	titleText->SetAlignment(GUIText::Alignment::Left);
	titleText->SetText(L"Member List");

	m_panel->AddChild(m_titleText);

	m_leavePartyButton = std::make_shared<SceneObject>();
	auto leavePartyButtonRenderer = m_leavePartyButton->AddComponent<GUIButton>();
	m_leavePartyButton->GetTransform()->SetLocalPosition(Vector3(220.0f, 140.0f, 0.0f));
	leavePartyButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
	leavePartyButtonRenderer->SetSize(Vector2(30, 30));
	leavePartyButtonRenderer->SetClickCallback([]() {
		// TODO: 파티 리셋, 또는 탈퇴 패킷, 다만 현재 파퀘 진행중이라면 여기서도 좀 검증 필요
		Send(Create_c2s_PARTY_OUT());
	});

	auto leavePartyButtonText = m_leavePartyButton->AddComponent<GUIText>();
	leavePartyButtonText->SetFont(font);
	leavePartyButtonText->SetRaycastTarget(false);
	leavePartyButtonText->SetAlignment(GUIText::Alignment::Center);
	leavePartyButtonText->SetText(L"Leave");

	m_panel->AddChild(m_leavePartyButton);

	for (int i = 0; i < 4; ++i)
	{
		float y = i * -70.0f + 90.0f;
		auto& partyGUI = m_partyPanels.emplace_back();

		partyGUI.Panel = std::make_shared<SceneObject>();
		partyGUI.Panel->GetTransform()->SetLocalPositionY(y);
		auto panelRenderer = partyGUI.Panel->AddComponent<GUIImage>();
		panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
		panelRenderer->SetSize(Vector2(460.0f, 60.0f));
		m_panel->AddChild(partyGUI.Panel);

		partyGUI.PartyMemberIDText = std::make_shared<SceneObject>();
		auto partyMemberIDText = partyGUI.PartyMemberIDText->AddComponent<GUIText>();
		partyMemberIDText->SetFont(font);
		partyGUI.PartyMemberIDText->GetTransform()->SetLocalPosition(Vector3(-220.0f, 0.0f, 0.0f));
		partyMemberIDText->SetRaycastTarget(false);
		partyMemberIDText->SetText(L"Member ID");
		partyMemberIDText->SetAlignment(GUIText::Alignment::Left);
		partyGUI.Panel->AddChild(partyGUI.PartyMemberIDText);
	}

	m_panel->SetActive(false);
}

void PartyStatusGUI::InitializeContents(const std::vector<uint32_t>& table)
{
	m_partyMemberIDsCache = table;
	UpdatePartyPanels();
	m_panel->SetActive(true);
}

void PartyStatusGUI::AddPartyMember(uint32_t partyMemberID)
{
	m_partyMemberIDsCache.push_back(partyMemberID);
	UpdatePartyPanels();
}

void PartyStatusGUI::RemovePartyMember(uint32_t partyMemberID)
{
	auto it = std::remove(m_partyMemberIDsCache.begin(), m_partyMemberIDsCache.end(), partyMemberID);
	if (it != m_partyMemberIDsCache.end())
	{
		m_partyMemberIDsCache.erase(it, m_partyMemberIDsCache.end());
		UpdatePartyPanels();
	}
}

void PartyStatusGUI::DisablePartyPanel()
{
	m_panel->SetActive(false);
}

void PartyStatusGUI::UpdatePartyPanels()
{
	for (size_t i = 0; i < m_partyPanels.size(); ++i)
	{
		if (i < m_partyMemberIDsCache.size())
		{
			m_partyPanels[i].PartyMemberIDText->SetActive(true);
			auto partyMemberIDText = m_partyPanels[i].PartyMemberIDText->GetComponent<GUIText>();
			partyMemberIDText->SetText(L"Member ID: " + std::to_wstring(m_partyMemberIDsCache[i]));
		}
		else
		{
			m_partyPanels[i].PartyMemberIDText->SetActive(false);
		}
	}
}
