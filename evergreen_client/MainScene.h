#pragma once

#include "pch.h"

class PopupGUIManager;
class TransitionOverlayGUI;

class MainScene : public udsdx::Scene
{
public:
	MainScene();
	void OnAttach() override;

	// 멀티플레이어 버튼을 클릭했을 때 호출되는 함수
	void OnLoginDialog();
	// 게임 시작 시 캐릭터를 선택하는 단계
	void EnterCharacterSelection(bool enter);
	void OnLoginResult(Nagox::Enum::LOGIN_RESULT result, unsigned int characterType);
	bool WaitRegisterResult();

	void TransitionEnterGame();
	void EnterGame();
	void ExitGame();

private:
	std::shared_ptr<udsdx::SceneObject> m_environmentObject;
	std::shared_ptr<udsdx::SceneObject> m_environmentLightObj;
	std::shared_ptr<udsdx::SceneObject> m_skyboxObj;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuCameraObject;
	std::shared_ptr<udsdx::SceneObject> m_characterSelectObject;

	std::shared_ptr<udsdx::SceneObject> m_interfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuObj;
	std::shared_ptr<udsdx::SceneObject> m_playerSelectObj;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchObj;

	PopupGUIManager* m_popupGUIManager;
	TransitionOverlayGUI* m_transitionOverlayGUI;

	bool m_firstLoginAttempt = true;
	unsigned int m_currentCharacterType = 0;
	unsigned int m_currentChannelID = 0;
	bool m_needCharacterSelection = false;
	bool m_register_account = false;
	bool m_isRegisterSuccess = false;


	std::string m_userId = "localuser";
	std::string m_userPw;
	std::string m_class_type;
};