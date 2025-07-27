#pragma once

#include "pch.h"

class GamePauseGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnActive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }
	void SetChannelSwitchGUI(const std::shared_ptr<udsdx::SceneObject>& channelSwitchGUI) { m_channelSwitchGUI = channelSwitchGUI; }

private:
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchGUI;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::array<std::shared_ptr<udsdx::SceneObject>, 2> m_verticalPanel;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchButton;
	std::shared_ptr<udsdx::SceneObject> m_resumeButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	std::function<void()> m_exitGameCallback;

	float m_activeFactor = 0.0f;
};