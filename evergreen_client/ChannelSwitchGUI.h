#pragma once

#include "pch.h"

class ChannelSwitchGUI : public udsdx::Component
{
private:
	struct ChannelButton
	{
		std::shared_ptr<udsdx::SceneObject> ButtonPanel;
		std::shared_ptr<udsdx::SceneObject> ChannelText;
		std::shared_ptr<udsdx::SceneObject> StatusImage;
	};

public:
	void OnInitialize() override;
	void OnActive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetPanelGraphic(bool isEnter);
	void SwitchChannelPage(int page);
	void InitializeChannel(int channel);
	void SelectChannel(int channel);
	void SetChannelSelectedCallback(std::function<void(int)> callback) { m_channelSelectedCallback = callback; }

private:
	constexpr static int PageCount = 3;
	constexpr static int PageRows = 4;
	constexpr static int PageColumns = 5;

	int m_currentPage = 0;
	int m_currentChannel = -1;

	std::shared_ptr<udsdx::SceneObject> m_background;
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_incrementPageButton;
	std::shared_ptr<udsdx::SceneObject> m_decrementPageButton;
	std::vector<ChannelButton> m_partyPanels;

	std::function<void(int)> m_channelSelectedCallback;
};