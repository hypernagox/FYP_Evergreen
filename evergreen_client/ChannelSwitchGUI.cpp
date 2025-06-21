#include "pch.h"
#include "ChannelSwitchGUI.h"
#include "GameGUIFacade.h"
#include "LogFloatGUI.h"
#include "NetworkMgr.h"

using namespace udsdx;

ChannelSwitchGUI::ChannelSwitchGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	auto font = INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont"));

	m_background = std::make_shared<SceneObject>();
	auto backgroundRenderer = m_background->AddComponent<GUIImage>();
	backgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\common_background.png")));
	backgroundRenderer->SetSize(Vector2::One * 8192.0f);
	object->AddChild(m_background);

	m_panel = std::make_shared<SceneObject>();
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\move_channel.png")), true);
	object->AddChild(m_panel);

	for (int row = 0; row < PageRows; ++row)
	{
		for (int col = 0; col < PageColumns; ++col)
		{
			ChannelButton button;
			button.ButtonPanel = std::make_shared<SceneObject>();
			auto buttonRenderer = button.ButtonPanel->AddComponent<GUIButton>();
			button.ButtonPanel->GetTransform()->SetLocalPosition(Vector3(-340.0f + col * 180.0f, 100.0f - row * 80.0f, 0.0f));
			buttonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\channel.png")), true);
			m_panel->AddChild(button.ButtonPanel);

			button.ChannelText = std::make_shared<SceneObject>();
			button.ChannelText->GetTransform()->SetLocalPosition(Vector3(-80.0f, 7.5f, 0.0f));
			auto channelText = button.ChannelText->AddComponent<GUIText>();
			channelText->SetFont(font);
			channelText->SetRaycastTarget(false);
			channelText->SetAlignment(GUIText::Alignment::Left);
			channelText->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
			button.ButtonPanel->AddChild(button.ChannelText);

			button.StatusImage = std::make_shared<SceneObject>();
			auto statusImageRenderer = button.StatusImage->AddComponent<GUIImage>();
			statusImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\server_state_01.png")), true);
			statusImageRenderer->SetRaycastTarget(false);
			button.StatusImage->GetTransform()->SetLocalPosition(Vector3(0.0f, -12.5f, 0));
			button.ButtonPanel->AddChild(button.StatusImage);

			m_partyPanels.push_back(button);
		}
	}

	m_incrementPageButton = std::make_shared<SceneObject>();
	auto incrementPageButtonRenderer = m_incrementPageButton->AddComponent<GUIButton>();
	m_incrementPageButton->GetTransform()->SetLocalPosition(Vector3(100.0f, -200.0f, 0.0f));
	incrementPageButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\channel_arrow_R.png")), true);
	incrementPageButtonRenderer->SetClickCallback([this]() { SwitchChannelPage((m_currentPage + 1) % PageCount); });
	m_panel->AddChild(m_incrementPageButton);

	m_decrementPageButton = std::make_shared<SceneObject>();
	auto decrementPageButtonRenderer = m_decrementPageButton->AddComponent<GUIButton>();
	m_decrementPageButton->GetTransform()->SetLocalPosition(Vector3(-100.0f, -200.0f, 0.0f));
	decrementPageButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\channel_arrow_L.png")), true);
	decrementPageButtonRenderer->SetClickCallback([this]() { SwitchChannelPage((m_currentPage - 1 + PageCount) % PageCount); });
	m_panel->AddChild(m_decrementPageButton);

	SwitchChannelPage(m_currentPage);
}

void ChannelSwitchGUI::Update(const Time& time, Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::Escape))
		GetSceneObject()->SetActive(false);
}

void ChannelSwitchGUI::SetPanelGraphic(bool isEnter)
{
	auto uiRenderer = m_panel->AddComponent<GUIImage>();
	if (isEnter)
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\choose_channel.png")), true);
	else
		uiRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\move_channel.png")), true);
}

void ChannelSwitchGUI::SwitchChannelPage(int page)
{
	m_currentPage = page;

	for (int i = 0; i < m_partyPanels.size(); ++i)
	{
		auto& button = m_partyPanels[i];
		int channelIndex = page * (PageRows * PageColumns) + i;

		button.ChannelText->GetComponent<GUIText>()->SetText(std::format(L"Ch. {0:02}", channelIndex + 1));

		auto buttonComponent = button.ButtonPanel->GetComponent<GUIButton>();
		if (channelIndex == m_currentChannel)
			buttonComponent->SetInteractable(false);
		else
		{
			buttonComponent->SetInteractable(true);
			buttonComponent->SetClickCallback([this, channelIndex]() { SelectChannel(channelIndex); });
		}

		auto statusImageRenderer = button.StatusImage->GetComponent<GUIImage>();
		if (channelIndex < 4)
			statusImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\server_state_01.png")));
		else if (channelIndex < 8)
			statusImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\server_state_02.png")));
		else if (channelIndex < 12)
			statusImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\server_state_03.png")));
		else
			statusImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\channel\\server_state_04.png")));
	}
}

void ChannelSwitchGUI::SelectChannel(int channel)
{
	m_currentChannel = channel;
	if (m_channelSelectedCallback)
		m_channelSelectedCallback(channel);
}
