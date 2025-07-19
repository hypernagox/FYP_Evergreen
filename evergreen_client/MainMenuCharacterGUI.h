#pragma once

#include "pch.h"

class MainMenuCharacterGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void SetCharacterShowCallback(std::function<void(unsigned int)> callback) { m_characterShowCallback = callback; }
	void SetEnterGameCallback(std::function<void(unsigned int)> callback) { m_enterGameCallback = callback; }
	void EnterGame();
	void SetSelectIndex(unsigned int index);

private:
	unsigned int m_selectIndex = 0;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_nextButton;
	std::shared_ptr<udsdx::SceneObject> m_prevButton;
	std::shared_ptr<udsdx::SceneObject> m_selectButton;

	std::function<void(unsigned int)> m_characterShowCallback;
	std::function<void(unsigned int)> m_enterGameCallback;
};