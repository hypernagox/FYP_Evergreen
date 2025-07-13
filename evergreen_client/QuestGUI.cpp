#include "pch.h"
#include "QuestGUI.h"
#include "PopupGUIManager.h"
#include "GameGUIFacade.h"
#include "LogFloatGUI.h"
#include "RequestPopupGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void QuestGUI::OnInitialize()
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

#pragma region Quest List Panel
	{
		m_questListPanel = std::make_shared<SceneObject>();
		m_questListPanel->SetActive(false);
		auto uiRenderer = m_questListPanel->AddComponent<GUIImage>();
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_questlist.png")), true);
		GetSceneObject()->AddChild(m_questListPanel);

		auto incrementQuestButton = std::make_shared<SceneObject>();
		m_incrementQuestButton = incrementQuestButton->AddComponent<GUISimpleButton>();
		incrementQuestButton->GetTransform()->SetLocalPosition(Vector3(80.0f, -250.0f, 0.0f));
		m_incrementQuestButton->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_arrow_R.png")), true);
		m_incrementQuestButton->SetClickCallback([&]() {
			if (GetNumQuestPages() <= 0)
				return;
			UpdateQuestPage((m_currentQuestPage + 1) % GetNumQuestPages());
			});
		m_questListPanel->AddChild(incrementQuestButton);

		auto decrementQuestButton = std::make_shared<SceneObject>();
		m_decrementQuestButton = decrementQuestButton->AddComponent<GUISimpleButton>();
		decrementQuestButton->GetTransform()->SetLocalPosition(Vector3(-80.0f, -250.0f, 0.0f));
		m_decrementQuestButton->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_arrow_L.png")), true);
		m_decrementQuestButton->SetClickCallback([&]() {
			if (GetNumQuestPages() <= 0)
				return;
			UpdateQuestPage((m_currentQuestPage - 1 + GetNumQuestPages()) % GetNumQuestPages());
			});
		m_questListPanel->AddChild(decrementQuestButton);

		for (int i = 0; i < SIZE_QUEST_LIST; ++i)
		{
			auto questContentObj = std::make_shared<SceneObject>();
			auto questImageObj = std::make_shared<SceneObject>();
			auto questNameObj = std::make_shared<SceneObject>();
			auto questDescObj = std::make_shared<SceneObject>();

			m_questList[i].ButtonPanel = questContentObj->AddComponent<GUISimpleButton>();
			m_questList[i].ButtonPanel->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_slot_1.png")), true);

			m_questList[i].IconImage = questImageObj->AddComponent<GUIImage>();
			m_questList[i].IconImage->SetRaycastTarget(false);
			m_questList[i].IconImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_monster_fox.png")), true);

			m_questList[i].NameText = questNameObj->AddComponent<GUIText>();
			m_questList[i].NameText->SetFont(font);
			m_questList[i].NameText->GetTransform()->SetLocalPosition(Vector3(-120.0f, 16.0f, 0.0f));
			m_questList[i].NameText->GetTransform()->SetLocalScale(0.8f);
			m_questList[i].NameText->SetRaycastTarget(false);
			m_questList[i].NameText->SetAlignment(GUIText::Alignment::Left);
			m_questList[i].NameText->SetText(L"## Quest Name ##");
			m_questList[i].NameText->SetColor(Color(0.125f, 0.125f, 0.125f, 1.0f));

			m_questList[i].DescriptionText = questDescObj->AddComponent<GUIText>();
			m_questList[i].DescriptionText->SetFont(font);
			m_questList[i].DescriptionText->GetTransform()->SetLocalPosition(Vector3(-125.0f, 0.0f, 0.0f));
			m_questList[i].DescriptionText->GetTransform()->SetLocalScale(0.65f);
			m_questList[i].DescriptionText->SetRaycastTarget(false);
			m_questList[i].DescriptionText->SetAlignment(GUIText::Alignment::UpperLeft);
			m_questList[i].DescriptionText->SetText(L"## Quest Description ##");
			m_questList[i].DescriptionText->SetColor(Color(0.125f, 0.125f, 0.125f, 1.0f));

			questContentObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 140.0f - i * 80.0f, 0.0f));
			questImageObj->GetTransform()->SetLocalPosition(Vector3(-160.0f, 0.0f, 0.0f));

			questContentObj->AddChild(questImageObj);
			questContentObj->AddChild(questNameObj);
			questContentObj->AddChild(questDescObj);

			m_questListPanel->AddChild(questContentObj);
		}

		auto exitButton = std::make_shared<SceneObject>();
		auto exitButtonRenderer = exitButton->AddComponent<GUISimpleButton>();
		exitButton->GetTransform()->SetLocalPosition(Vector3(175.0f, 250.0f, 0.0f));
		exitButtonRenderer->SetSize(Vector2(50.0f, 50.0f));
		exitButtonRenderer->SetClickCallback([&]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
			});
		m_questListPanel->AddChild(exitButton);
	}
#pragma endregion

#pragma region Party List Panel
	{
		m_partyListPanel = std::make_shared<SceneObject>();
		m_partyListPanel->SetActive(false);
		auto uiRenderer = m_partyListPanel->AddComponent<GUIImage>();
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_find.png")), true);
		GetSceneObject()->AddChild(m_partyListPanel);

		m_standByText = std::make_shared<SceneObject>();
		m_standByText->SetActive(false);
		auto standByText = m_standByText->AddComponent<GUIText>();
		standByText->SetFont(font);
		standByText->SetRaycastTarget(false);
		standByText->SetAlignment(GUIText::Alignment::Center);
		standByText->SetText(L". . .");
		m_partyListPanel->AddChild(m_standByText);

		auto selectedQuestObj = std::make_shared<SceneObject>();
		m_selectedQuestText = selectedQuestObj->AddComponent<GUIText>();
		m_selectedQuestText->SetFont(font);
		m_selectedQuestText->GetTransform()->SetLocalPosition(Vector3(-180.0f, 182.0f, 0.0f));
		m_selectedQuestText->SetRaycastTarget(false);
		m_selectedQuestText->SetAlignment(GUIText::Alignment::Left);
		m_selectedQuestText->SetText(L"## Selected Quest ##");
		m_selectedQuestText->SetColor(Color(0.125f, 0.125f, 0.125f, 1.0f));
		m_partyListPanel->AddChild(selectedQuestObj);

		auto incrementQuestButton = std::make_shared<SceneObject>();
		m_incrementPartyButton = incrementQuestButton->AddComponent<GUISimpleButton>();
		incrementQuestButton->GetTransform()->SetLocalPosition(Vector3(80.0f, -180.0f, 0.0f));
		m_incrementPartyButton->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_arrow_R.png")), true);
		m_incrementPartyButton->SetClickCallback([&]() {
			if (GetNumPartyPages() <= 0)
				return;
			UpdatePartyPage((m_currentPartyPage + 1) % GetNumPartyPages());
			});
		m_partyListPanel->AddChild(incrementQuestButton);

		auto decrementQuestButton = std::make_shared<SceneObject>();
		m_decrementPartyButton = decrementQuestButton->AddComponent<GUISimpleButton>();
		decrementQuestButton->GetTransform()->SetLocalPosition(Vector3(-80.0f, -180.0f, 0.0f));
		m_decrementPartyButton->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_arrow_L.png")), true);
		m_decrementPartyButton->SetClickCallback([&]() {
			if (GetNumPartyPages() <= 0)
				return;
			UpdatePartyPage((m_currentPartyPage - 1 + GetNumPartyPages()) % GetNumPartyPages());
			});
		m_partyListPanel->AddChild(decrementQuestButton);

		auto partyCreateButton = std::make_shared<SceneObject>();
		auto partyCreateButtonRenderer = partyCreateButton->AddComponent<GUISimpleButton>();
		partyCreateButton->GetTransform()->SetLocalPosition(Vector3(0.0f, -240.0f, 0.0f));
		partyCreateButtonRenderer->SetClickCallback([this]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Append(GetSceneObject(), m_partyCreatePanel);
			});
		partyCreateButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_create_slot.png")), true);
		m_partyListPanel->AddChild(partyCreateButton);

		for (int i = 0; i < SIZE_PARTY_LIST; ++i)
		{
			auto questContentObj = std::make_shared<SceneObject>();
			auto questImageObj = std::make_shared<SceneObject>();
			auto partyNameObj = std::make_shared<SceneObject>();
			auto partyLeaderObj = std::make_shared<SceneObject>();

			m_partyList[i].ButtonPanel = questContentObj->AddComponent<GUISimpleButton>();
			m_partyList[i].ButtonPanel->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_slot_1.png")), true);

			m_partyList[i].IconImage = questImageObj->AddComponent<GUIImage>();
			m_partyList[i].IconImage->SetRaycastTarget(false);
			m_partyList[i].IconImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_monster_fox.png")), true);

			m_partyList[i].NameText = partyNameObj->AddComponent<GUIText>();
			m_partyList[i].NameText->SetFont(font);
			m_partyList[i].NameText->GetTransform()->SetLocalPosition(Vector3(-120.0f, 16.0f, 0.0f));
			m_partyList[i].NameText->GetTransform()->SetLocalScale(0.8f);
			m_partyList[i].NameText->SetRaycastTarget(false);
			m_partyList[i].NameText->SetAlignment(GUIText::Alignment::Left);
			m_partyList[i].NameText->SetText(L"## Party Name ##");
			m_partyList[i].NameText->SetColor(Color(0.125f, 0.125f, 0.125f, 1.0f));

			m_partyList[i].DescriptionText = partyLeaderObj->AddComponent<GUIText>();
			m_partyList[i].DescriptionText->SetFont(font);
			m_partyList[i].DescriptionText->GetTransform()->SetLocalPosition(Vector3(-125.0f, 0.0f, 0.0f));
			m_partyList[i].DescriptionText->GetTransform()->SetLocalScale(0.65f);
			m_partyList[i].DescriptionText->SetRaycastTarget(false);
			m_partyList[i].DescriptionText->SetAlignment(GUIText::Alignment::UpperLeft);
			m_partyList[i].DescriptionText->SetText(L"파티장: ## Party Leader Name ##\n현재인원 ##");
			m_partyList[i].DescriptionText->SetColor(Color(0.125f, 0.125f, 0.125f, 1.0f));

			questContentObj->GetTransform()->SetLocalPosition(Vector3(0.0f, 120.0f - i * 80.0f, 0.0f));
			questImageObj->GetTransform()->SetLocalPosition(Vector3(-160.0f, 0.0f, 0.0f));

			questContentObj->AddChild(questImageObj);
			questContentObj->AddChild(partyNameObj);
			questContentObj->AddChild(partyLeaderObj);

			m_partyListPanel->AddChild(questContentObj);
		}

		auto exitButton = std::make_shared<SceneObject>();
		auto exitButtonRenderer = exitButton->AddComponent<GUISimpleButton>();
		exitButton->GetTransform()->SetLocalPosition(Vector3(175.0f, 250.0f, 0.0f));
		exitButtonRenderer->SetSize(Vector2(50.0f, 50.0f));
		exitButtonRenderer->SetClickCallback([&]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
			});
		m_partyListPanel->AddChild(exitButton);
	}
#pragma endregion

#pragma region Party Create Panel
	{
		m_partyCreatePanel = std::make_shared<SceneObject>();
		m_partyCreatePanel->SetActive(false);
		auto uiRenderer = m_partyCreatePanel->AddComponent<GUIImage>();
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_create.png")), true);
		GetSceneObject()->AddChild(m_partyCreatePanel);

		auto createButton = std::make_shared<SceneObject>();
		auto createButtonRenderer = createButton->AddComponent<GUISimpleButton>();
		createButton->GetTransform()->SetLocalPosition(Vector3(-100.0f, -70.0f, 0.0f));
		createButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_opinion_yes.png")), true);
		createButtonRenderer->SetClickCallback([&]() {
			Send(Create_c2s_REGISTER_PARTY_QUEST(m_selectedQuestID));
			RequestPartyList(m_selectedQuestID);
			INSTANCE(GameGUIFacade)->LogFloat->AddText(L"퀘스트 \'" + PartyQuestTable::GetPartyQuestStr(m_selectedQuestID) + L"\'의 파티를 생성했습니다.");
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
			});
		m_partyCreatePanel->AddChild(createButton);

		auto cancelButton = std::make_shared<SceneObject>();
		auto cancelButtonRenderer = cancelButton->AddComponent<GUISimpleButton>();
		cancelButton->GetTransform()->SetLocalPosition(Vector3(100.0f, -70.0f, 0.0f));
		cancelButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\party_ui\\party_opinion_no.png")), true);
		cancelButtonRenderer->SetClickCallback([&]() {
			GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Pop(GetSceneObject());
			});
		m_partyCreatePanel->AddChild(cancelButton);
	}
#pragma endregion

	FetchQuestList();
	UpdateQuestPage(0);
	UpdatePartyPage(0);
}

void QuestGUI::RequestPartyList(int questID)
{
	Send(Create_c2s_ACQUIRE_PARTY_LIST(questID));

	m_standByText->SetActive(true);
	for (const auto& partyGUI : m_partyList)
		partyGUI.ButtonPanel->SetActive(false);
}

void QuestGUI::FetchQuestList()
{
	m_numQuests = static_cast<int>(PartyQuestTable::GetPartyQuestSize());

	m_incrementQuestButton->SetInteractable(GetNumQuestPages() > 1);
	m_decrementQuestButton->SetInteractable(GetNumQuestPages() > 1);
}

void QuestGUI::FetchPartyList(const std::vector<uint32_t>& table)
{
	m_partyTableCache = table;
	m_numParties = static_cast<int>(table.size());

	m_standByText->SetActive(false);
	for (const auto& partyGUI : m_partyList)
		partyGUI.ButtonPanel->SetActive(true);

	m_incrementPartyButton->SetInteractable(GetNumPartyPages() > 1);
	m_decrementPartyButton->SetInteractable(GetNumPartyPages() > 1);

	UpdatePartyPage(m_currentPartyPage);
}

void QuestGUI::UpdateQuestPage(int page)
{
	m_currentQuestPage = page;

	for (int i = 0; i < SIZE_QUEST_LIST; ++i)
	{
		int questIndex = page * SIZE_QUEST_LIST + i;
		if (questIndex < m_numQuests)
		{
			m_questList[i].ButtonPanel->SetInteractable(true);
			m_questList[i].ButtonPanel->SetClickCallback([this, questIndex]() {
				m_selectedQuestID = questIndex;
				RequestPartyList(questIndex);
				m_selectedQuestText->SetText(PartyQuestTable::GetPartyQuestStr(questIndex));
				GetSceneObject()->GetComponentInParent<PopupGUIManager>()->Append(GetSceneObject(), m_partyListPanel);
			});
			m_questList[i].NameText->SetText(PartyQuestTable::GetPartyQuestStr(questIndex));
			m_questList[i].DescriptionText->SetText(PartyQuestTable::GetPartyQuestDescription(questIndex));
			m_questList[i].IconImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(PartyQuestTable::GetPartyQuestIcon(questIndex))));
		}
		else
		{
			m_questList[i].ButtonPanel->SetInteractable(false);
			m_questList[i].NameText->SetText(L"");
			m_questList[i].DescriptionText->SetText(L"");
			m_questList[i].IconImage->SetTexture(nullptr);
		}
	}
}

void QuestGUI::UpdatePartyPage(int page)
{
	m_currentPartyPage = page;

	uint32_t sessionID = NetHelper::NetworkMgr::GetInst()->GetSessionID();

	for (int i = 0; i < SIZE_PARTY_LIST; ++i)
	{
		int partyIndex = page * SIZE_PARTY_LIST + i;
		if (partyIndex < m_numParties)
		{
			if (m_partyTableCache[i] == sessionID)
			{
				m_partyList[i].DescriptionText->SetText(L"Party Member ID: " + std::to_wstring(m_partyTableCache[i]) + L" (You)");
				m_partyList[i].ButtonPanel->SetInteractable(false);
			}
			else
			{
				auto funcSendJoinRequest = [this, id = m_partyTableCache[i]]() {
					Send(Create_c2s_PARTY_JOIN_REQUEST(id, m_selectedQuestID));
					INSTANCE(GameGUIFacade)->LogFloat->AddText(std::to_wstring(id) + L" 의 파티에 참가 신청을 보냈습니다.");
					};
				m_partyList[i].DescriptionText->SetText(L"Party Member ID: " + std::to_wstring(m_partyTableCache[i]));
				m_partyList[i].ButtonPanel->SetInteractable(true);
				m_partyList[i].ButtonPanel->SetClickCallback([funcSendJoinRequest, id = m_partyTableCache[i]]() {
					INSTANCE(GameGUIFacade)->RequestPopup->ShowPopup(L"참가 신청", std::to_wstring(id) + L" 의 파티에 참가 신청을 보내시겠습니까?", funcSendJoinRequest, nullptr);
					});
			}
			m_partyList[i].IconImage->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(PartyQuestTable::GetPartyQuestIcon(m_selectedQuestID))));
		}
		else
		{
			m_partyList[i].ButtonPanel->SetInteractable(false);
			m_partyList[i].NameText->SetText(L"");
			m_partyList[i].DescriptionText->SetText(L"");
			m_partyList[i].IconImage->SetTexture(nullptr);
		}
	}
}

void PartyQuestTable::InitPartyQuestTable(const std::wstring_view path) noexcept
{
	const auto party_quest_table_path = RESOURCE_PATH(path) + L"\\party_quest_table\\PartyQuestTable.json";

	std::ifstream file_stream{ party_quest_table_path };
	if (!file_stream)
	{
		std::cout << "파티퀘스트 테이블 경로 오류\n";
		return;
	}

	nlohmann::json j;
	file_stream >> j;

	for (size_t i = 0; i < j.size(); ++i)
	{
		const std::string& utf8_str = j[i]["name"];
		std::wstring wide_str = Common::DataRegistry::Str2Wstr(utf8_str);
		m_name_to_id[wide_str] = static_cast<int>(i);
		m_id_to_name[static_cast<int>(i)] = std::move(wide_str);
		m_id_to_description[static_cast<int>(i)] = Common::DataRegistry::Str2Wstr(j[i]["description"]);
		m_id_to_icon[static_cast<int>(i)] = Common::DataRegistry::Str2Wstr(j[i]["icon"]);
	}
}