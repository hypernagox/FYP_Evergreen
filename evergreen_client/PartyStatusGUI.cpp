#include "pch.h"
#include "PartyStatusGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void PartyStatusGUI::OnInitialize()
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

	m_panel = SceneObject::MakeShared();
	m_panel->GetTransform()->SetLocalPosition(Vector3(1048.0f, -406.0f, 0.0f));
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_member_list.png")), true);

	GetSceneObject()->AddChild(m_panel);

	m_leavePartyButton = SceneObject::MakeShared();
	auto leavePartyButtonRenderer = m_leavePartyButton->AddComponent<GUISimpleButton>();
	m_leavePartyButton->GetTransform()->SetLocalPosition(Vector3(145.0f, 83.0f, 0.0f));
	leavePartyButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\quest_box.png")));
	leavePartyButtonRenderer->SetSize(Vector2(110.0f, 35.0f));
	leavePartyButtonRenderer->SetClickCallback([]() {
		// TODO: 파티 리셋, 또는 탈퇴 패킷, 다만 현재 파퀘 진행중이라면 여기서도 좀 검증 필요
		Send(Create_c2s_PARTY_OUT());
		});

	auto leavePartyButtonText = m_leavePartyButton->AddComponent<GUIText>();
	leavePartyButtonText->SetFont(font);
	leavePartyButtonText->SetRaycastTarget(false);
	leavePartyButtonText->SetAlignment(GUIText::Alignment::Center);
	leavePartyButtonText->SetText(L"파티 탈퇴");

	m_panel->AddChild(m_leavePartyButton);

	for (int i = 0; i < 4; ++i)
	{
		float y = i * -38.0f + 38.0f;
		auto& partyGUI = m_partyPanels.emplace_back();

		partyGUI.Panel = SceneObject::MakeShared();
		partyGUI.Panel->GetTransform()->SetLocalPositionY(y);
		//auto panelRenderer = partyGUI.Panel->AddComponent<GUIImage>();
		//panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
		//panelRenderer->SetSize(Vector2(460.0f, 60.0f));
		m_panel->AddChild(partyGUI.Panel);

		partyGUI.PartyMemberIDText = SceneObject::MakeShared();
		auto partyMemberIDText = partyGUI.PartyMemberIDText->AddComponent<GUIText>();
		partyMemberIDText->SetFont(font);
		partyGUI.PartyMemberIDText->GetTransform()->SetLocalPosition(Vector3(-200.0f, 0.0f, 0.0f));
		partyMemberIDText->SetRaycastTarget(false);
		partyMemberIDText->SetText(L"Member ID");
		partyMemberIDText->SetAlignment(GUIText::Alignment::Left);
		partyGUI.Panel->AddChild(partyGUI.PartyMemberIDText);
	}

	m_panel->SetActive(false);

	m_dialogPanel = SceneObject::MakeShared();
	m_dialogPanel->GetTransform()->SetLocalPosition(Vector3(1048.0f, -170.0f, 0.0f));
	auto dialogPanelRenderer = m_dialogPanel->AddComponent<GUIImage>();
	dialogPanelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_dialogue.png")), true);

	m_dialogText = SceneObject::MakeShared();
	auto dialogTextRenderer = m_dialogText->AddComponent<GUIText>();
	dialogTextRenderer->SetFont(font);
	dialogTextRenderer->SetRaycastTarget(false);
	dialogTextRenderer->SetText(L"퀘스트 클리어!\nESC 키를 눌러 마우스를 활성화하세요.");
	dialogTextRenderer->SetAlignment(GUIText::Alignment::UpperLeft);
	m_dialogText->GetTransform()->SetLocalPosition(Vector3(-190.0f, 60.0f, 0.0f));
	m_dialogPanel->AddChild(m_dialogText);

	m_dialogEnterButton = SceneObject::MakeShared();
	auto dialogEnterButtonRenderer = m_dialogEnterButton->AddComponent<GUISimpleButton>();
	m_dialogEnterButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -52.0f, 0.0f));
	dialogEnterButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\button_enter_quest.png")), true);
	dialogEnterButtonRenderer->SetClickCallback([this]() {
		RequestQuestStart();
	});
	m_dialogPanel->AddChild(m_dialogEnterButton);

	m_dialogExitButton = SceneObject::MakeShared();
	auto dialogExitButtonRenderer = m_dialogExitButton->AddComponent<GUISimpleButton>();
	m_dialogExitButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -52.0f, 0.0f));
	dialogExitButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\button_exit_quest.png")), true);
	dialogExitButtonRenderer->SetClickCallback([this]() {
		RequestQuestEnd();
	});
	m_dialogPanel->AddChild(m_dialogExitButton);

	m_dialogEnterButton->SetActive(true);
	m_dialogExitButton->SetActive(false);

	GetSceneObject()->AddChild(m_dialogPanel);
	m_dialogPanel->SetActive(false);

}

void PartyStatusGUI::InitializeContents(const std::vector<uint32_t>& table, const std::vector<std::wstring>& names)
{
	m_partyMemberIDsCache = table;
	m_partyMemberNamesCache = names;
	m_partyLeaderIndexCache = 0;
	UpdatePartyPanels();
	m_panel->SetActive(true);
	m_dialogPanel->SetActive(m_partyMemberIDsCache[m_partyLeaderIndexCache] == NetMgr(NetworkMgr)->GetSessionID());
	SetDialogPanelMode(false);
}

void PartyStatusGUI::AddPartyMember(uint32_t partyMemberID, std::wstring_view partyMemberName)
{
	m_partyMemberIDsCache.push_back(partyMemberID);
	m_partyMemberNamesCache.push_back(partyMemberName.data());
	UpdatePartyPanels();
}

void PartyStatusGUI::RemovePartyMember(uint32_t partyMemberID)
{
	auto it = std::find(m_partyMemberIDsCache.begin(), m_partyMemberIDsCache.end(), partyMemberID);
	if (it != m_partyMemberIDsCache.end())
	{
		auto index = std::distance(m_partyMemberIDsCache.begin(), it);
		m_partyMemberIDsCache.erase(m_partyMemberIDsCache.begin() + index);
		m_partyMemberNamesCache.erase(m_partyMemberNamesCache.begin() + index);
		UpdatePartyPanels();
	}
}

void PartyStatusGUI::SetPartyLeader(uint32_t partyLeaderID)
{
	auto it = std::find(m_partyMemberIDsCache.begin(), m_partyMemberIDsCache.end(), partyLeaderID);
	if (it != m_partyMemberIDsCache.end())
	{
		uint32_t index = static_cast<uint32_t>(std::distance(m_partyMemberIDsCache.begin(), it));
		if (index != m_partyLeaderIndexCache)
		{
			m_partyLeaderIndexCache = index;
			UpdatePartyPanels();
		}
	}
}

void PartyStatusGUI::DisablePartyPanel()
{
	m_panel->SetActive(false);
	m_dialogPanel->SetActive(false);
}

void PartyStatusGUI::OnQuestClear()
{
	m_dialogPanel->SetActive(m_partyMemberIDsCache[m_partyLeaderIndexCache] == NetMgr(NetworkMgr)->GetSessionID());
	SetDialogPanelMode(true);
}

void PartyStatusGUI::SetDialogPanelMode(bool isEndDialogue)
{
	if (isEndDialogue)
	{
		m_dialogEnterButton->SetActive(false);
		m_dialogExitButton->SetActive(true);
		m_dialogText->GetComponent<GUIText>()->SetText(L"퀘스트 클리어!\n(ESC 키로 마우스를 활성화하세요.)");
	}
	else
	{
		m_dialogEnterButton->SetActive(true);
		m_dialogExitButton->SetActive(false);
		m_dialogText->GetComponent<GUIText>()->SetText(L"퀘스트를 시작합니다.\n(ESC 키로 마우스를 활성화하세요.)");
	}
}

void PartyStatusGUI::RequestQuestStart()
{
	Send(Create_c2s_QUEST_START());
	m_dialogPanel->SetActive(false);
	//INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([]() { Send(Create_c2s_QUEST_START()); }, L"퀘스트를 시작하는 중 ...");
}

void PartyStatusGUI::RequestQuestEnd()
{
	Send(Create_c2s_QUEST_END());
	m_dialogPanel->SetActive(m_partyMemberIDsCache[m_partyLeaderIndexCache] == NetMgr(NetworkMgr)->GetSessionID());
	SetDialogPanelMode(false);
	//INSTANCE(GameGUIFacade)->TransitionOverlay->AppendTransition([]() { Send(Create_c2s_QUEST_END()); }, L"퀘스트를 종료하는 중 ...");
}

void PartyStatusGUI::UpdatePartyPanels()
{
	for (size_t i = 0; i < m_partyPanels.size(); ++i)
	{
		if (i < m_partyMemberIDsCache.size())
		{
			m_partyPanels[i].PartyMemberIDText->SetActive(true);
			auto partyMemberIDText = m_partyPanels[i].PartyMemberIDText->GetComponent<GUIText>();
			partyMemberIDText->SetText(m_partyMemberNamesCache[i]);
		}
		else
		{
			m_partyPanels[i].PartyMemberIDText->SetActive(false);
		}
	}
}
