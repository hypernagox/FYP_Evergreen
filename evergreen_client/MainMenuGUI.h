#pragma once

#include "pch.h"

class MainMenuGUI : public udsdx::Component
{
public:
	MainMenuGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void SetEnterGameCallback(std::function<void()> callback) { m_enterGameCallback = callback; }
	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_titleImage;
	std::shared_ptr<udsdx::SceneObject> m_playButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	std::function<void()> m_enterGameCallback;
	std::function<void()> m_exitGameCallback;
};