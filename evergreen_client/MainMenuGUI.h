#pragma once

#include "pch.h"

class MainMenuGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
    void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetEnterGameCallback(std::function<void()> callback) { m_enterGameCallback = callback; }
	void SetOptionsCallback(std::function<void()> callback) { m_optionsCallback = callback; }
	void SetExitGameCallback(std::function<void()> callback) { m_exitGameCallback = callback; }

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_verticalPanel;
	std::shared_ptr<udsdx::SceneObject> m_titleImage;
	std::shared_ptr<udsdx::SceneObject> m_versionText;
	std::shared_ptr<udsdx::SceneObject> m_playButton;
	std::shared_ptr<udsdx::SceneObject> m_optionsButton;
	std::shared_ptr<udsdx::SceneObject> m_exitButton;

	udsdx::GUIImage* m_backgroundImage = nullptr;

	std::function<void()> m_enterGameCallback;
	std::function<void()> m_optionsCallback;
	std::function<void()> m_exitGameCallback;

	float m_elapsedTime = 0.0f;
};