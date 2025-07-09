#pragma once

#include "pch.h"

class PopupGUIManager;
class TransitionOverlayGUI;

class MainScene : public udsdx::Scene
{
public:
	MainScene();
	void OnAttach() override;

	// 게임 시작 시 캐릭터를 선택하는 단계
	void EnterCharacterSelection();

	void EnterGame(unsigned int character);
	void ExitGame();

private:
	std::shared_ptr<udsdx::SceneObject> m_environmentObject;
	std::shared_ptr<udsdx::SceneObject> m_environmentLightObj;
	std::shared_ptr<udsdx::SceneObject> m_skyboxObj;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuCameraObject;

	std::shared_ptr<udsdx::SceneObject> m_interfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuObj;
	std::shared_ptr<udsdx::SceneObject> m_playerSelectObj;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchObj;

	PopupGUIManager* m_popupGUIManager;
	TransitionOverlayGUI* m_transitionOverlayGUI;

	int m_currentChannelID = 0;
};