#pragma once

#include "pch.h"

class MainMenuGUI : public udsdx::Component
{
public:
	MainMenuGUI(const std::shared_ptr<udsdx::SceneObject>& object);
    void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetEnterGameCallback(std::function<void()> callback) { m_enterGameCallback = callback; }
	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_verticalPanel;
	std::shared_ptr<udsdx::SceneObject> m_titleImage;
	std::shared_ptr<udsdx::SceneObject> m_playButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	udsdx::GUIImage* m_backgroundImage = nullptr;

	std::function<void()> m_enterGameCallback;
	std::function<void()> m_exitGameCallback;

	float m_elapsedTime = 0.0f;
};