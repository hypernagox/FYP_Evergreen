#pragma once

#include "pch.h"

class GamePauseGUI : public udsdx::Component
{
public:
	GamePauseGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void SetActivePanel(bool active);
	void ToggleActivePanel();
	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }
	void SetTogglePauseCallback(std::function<void(bool)> callback);

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_resumeButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	std::function<void()> m_exitGameCallback;
	std::function<void(bool)> m_togglePauseCallback;
};