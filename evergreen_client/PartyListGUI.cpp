#include "pch.h"
#include "PartyListGUI.h"
#include "GameGUIFacade.h"
#include "LogFloatGUI.h"

using namespace udsdx;

PartyListGUI::PartyListGUI(const std::shared_ptr<udsdx::SceneObject>& object) : udsdx::Component(object)
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

	m_panel = std::make_shared<SceneObject>();
	m_panel->GetTransform()->SetLocalPosition(Vector3(-0.0f, 360.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	uiRenderer->SetSize(Vector2(480.0f, 320.0f));
	object->AddChild(m_panel);

	m_selectedQuestText = std::make_shared<SceneObject>();
	auto selectedQuestText = m_selectedQuestText->AddComponent<GUIText>();
	selectedQuestText->SetFont(font);
	m_selectedQuestText->GetTransform()->SetLocalPosition(Vector3(-230.0f, 140.0f, 0.0f));
	selectedQuestText->SetRaycastTarget(false);
	selectedQuestText->SetAlignment(GUIText::Alignment::Left);
	m_panel->AddChild(m_selectedQuestText);

	m_incrementQuestButton = std::make_shared<SceneObject>();
	auto incrementQuestButtonRenderer = m_incrementQuestButton->AddComponent<GUIButton>();
	m_incrementQuestButton->GetTransform()->SetLocalPosition(Vector3(140.0f, 140.0f, 0.0f));
	incrementQuestButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
	incrementQuestButtonRenderer->SetSize(Vector2(30, 30));
	m_panel->AddChild(m_incrementQuestButton);

	auto incrementQuestButtonText = m_incrementQuestButton->AddComponent<GUIText>();
	incrementQuestButtonText->SetFont(font);
	incrementQuestButtonText->SetRaycastTarget(false);
	incrementQuestButtonText->SetAlignment(GUIText::Alignment::Center);
	incrementQuestButtonText->SetText(L"+");

	m_decrementQuestButton = std::make_shared<SceneObject>();
	auto decrementQuestButtonRenderer = m_decrementQuestButton->AddComponent<GUIButton>();
	m_decrementQuestButton->GetTransform()->SetLocalPosition(Vector3(180.0f, 140.0f, 0.0f));
	decrementQuestButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
	decrementQuestButtonRenderer->SetSize(Vector2(30, 30));
	m_panel->AddChild(m_decrementQuestButton);

	auto decrementQuestButtonText = m_decrementQuestButton->AddComponent<GUIText>();
	decrementQuestButtonText->SetFont(font);
	decrementQuestButtonText->SetRaycastTarget(false);
	decrementQuestButtonText->SetAlignment(GUIText::Alignment::Center);
	decrementQuestButtonText->SetText(L"-");

	m_refreshQuestButton = std::make_shared<SceneObject>();
	auto refreshQuestButtonRenderer = m_refreshQuestButton->AddComponent<GUIButton>();
	m_refreshQuestButton->GetTransform()->SetLocalPosition(Vector3(220.0f, 140, 0.0f));
	refreshQuestButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
	refreshQuestButtonRenderer->SetSize(Vector2(30, 30));
	m_panel->AddChild(m_refreshQuestButton);

	auto refreshQuestButtonText = m_refreshQuestButton->AddComponent<GUIText>();
	refreshQuestButtonText->SetFont(font);
	refreshQuestButtonText->SetRaycastTarget(false);
	refreshQuestButtonText->SetAlignment(GUIText::Alignment::Center);
	refreshQuestButtonText->SetText(L"R");

	m_standByText = std::make_shared<SceneObject>();
	auto standByText = m_standByText->AddComponent<GUIText>();
	standByText->SetFont(font);
	standByText->SetRaycastTarget(false);
	standByText->SetAlignment(GUIText::Alignment::Center);
	standByText->SetText(L". . .");
	m_panel->AddChild(m_standByText);

	for (int id = 0; id < 3; id++)
	{
		float y = id * -70.0f + 90.0f;
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
		partyMemberIDText->SetText(L"Party Leader ID");
		partyMemberIDText->SetAlignment(GUIText::Alignment::Left);
		partyGUI.Panel->AddChild(partyGUI.PartyMemberIDText);

		partyGUI.JoinButton = std::make_shared<SceneObject>();
		auto joinButtonRenderer = partyGUI.JoinButton->AddComponent<GUIButton>();
		partyGUI.JoinButton->GetTransform()->SetLocalPosition(Vector3(200.0f, 0.0f, 0.0f));
		joinButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
		joinButtonRenderer->SetSize(Vector2(60, 60));
		partyGUI.Panel->AddChild(partyGUI.JoinButton);
	}

	m_createPartyButton = std::make_shared<SceneObject>();
	auto createPartyButtonRenderer = m_createPartyButton->AddComponent<GUIButton>();
	m_createPartyButton->GetTransform()->SetLocalPosition(Vector3(200.0f, -120.0f, 0.0f));
	createPartyButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_complete.png")));
	createPartyButtonRenderer->SetSize(Vector2(60, 60));
	m_panel->AddChild(m_createPartyButton);

	m_createPartyText = std::make_shared<SceneObject>();
	auto createPartyButtonText = m_createPartyText->AddComponent<GUIText>();
	m_createPartyText->GetTransform()->SetLocalPosition(Vector3(-220.0f, -120.0f, 0.0f));
	createPartyButtonText->SetFont(font);
	createPartyButtonText->SetRaycastTarget(false);
	createPartyButtonText->SetAlignment(GUIText::Alignment::Left);
	createPartyButtonText->SetText(L"파티 생성");
	m_panel->AddChild(m_createPartyText);

	m_createPartyButton->GetComponent<GUIButton>()->SetClickCallback([this]() {
		Send(Create_c2s_REGISTER_PARTY_QUEST(m_currentQuestID));
		INSTANCE(GameGUIFacade)->LogFloat->AddText(L"퀘스트 " + std::to_wstring(m_currentQuestID) + L"의 파티를 생성했습니다.");
		UpdateQuestID(m_currentQuestID);
	});

	m_incrementQuestButton->GetComponent<GUIButton>()->SetClickCallback([this]() {
		m_currentQuestID = (m_currentQuestID + 1) % NumQuests;
		UpdateQuestID(m_currentQuestID);
	});
	m_decrementQuestButton->GetComponent<GUIButton>()->SetClickCallback([this]() {
		m_currentQuestID = (m_currentQuestID - 1 + NumQuests) % NumQuests;
		UpdateQuestID(m_currentQuestID);
	});
	m_refreshQuestButton->GetComponent<GUIButton>()->SetClickCallback([this]() {
		UpdateQuestID(m_currentQuestID);
	});

	UpdateQuestID(m_currentQuestID);
}

void PartyListGUI::UpdateContents(const std::vector<uint32_t>& table)
{
	m_standByText->SetActive(false);
	for (size_t i = 0; i < m_partyPanels.size(); ++i)
	{
		if (i < table.size())
		{
			m_partyPanels[i].Panel->SetActive(true);
			m_partyPanels[i].PartyMemberIDText->GetComponent<GUIText>()->SetText(L"Party Member ID: " + std::to_wstring(table[i]));
			m_partyPanels[i].JoinButton->GetComponent<GUIButton>()->SetClickCallback([this, id = table[i]]() {
				Send(Create_c2s_PARTY_JOIN_REQUEST(id, m_currentQuestID));
				INSTANCE(GameGUIFacade)->LogFloat->AddText(std::to_wstring(id) + L" 의 파티에 참가 신청을 보냈습니다.");
			});
		}
		else
		{
			m_partyPanels[i].Panel->SetActive(false);
		}
	}
}

void PartyListGUI::UpdateQuestID(int questID)
{
	m_selectedQuestText->GetComponent<GUIText>()->SetText(L"Selected Quest ID: " + std::to_wstring(questID));
	Send(Create_c2s_ACQUIRE_PARTY_LIST(questID));

	m_standByText->SetActive(true);
	for (const auto& partyGUI : m_partyPanels)
		partyGUI.Panel->SetActive(false);
}
