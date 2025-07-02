#pragma once

#include "pch.h"

class GamePauseGUI : public udsdx::Component
{
public:
	GamePauseGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }
	void SetChannelSwitchGUI(const std::shared_ptr<udsdx::SceneObject>& channelSwitchGUI) { m_channelSwitchGUI = channelSwitchGUI; }

private:
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchGUI;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchButton;
	std::shared_ptr<udsdx::SceneObject> m_resumeButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	std::function<void()> m_exitGameCallback;
};