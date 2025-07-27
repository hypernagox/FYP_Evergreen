#pragma once

#include "pch.h"

class PopupGUIManager;
class TransitionOverlayGUI;

class MainScene : public udsdx::Scene
{
public:
	MainScene();
	void OnAttach() override;
	void OnDetach() override;

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
	std::shared_ptr<udsdx::SceneObject> m_optionsObj;

	PopupGUIManager* m_popupGUIManager;
	TransitionOverlayGUI* m_transitionOverlayGUI;

	bool m_firstLoginAttempt = true;
	unsigned int m_currentCharacterType = 0;
	unsigned int m_currentChannelID = 0;
	bool m_needCharacterSelection = false;
	bool m_register_account = false;

	std::unique_ptr<SoundEffectInstance> m_bgmSound;

	enum class RegisterStatus
	{
		None = 0,        // 로그인 이전 상태
		Success = 1,     // 로그인 성공 상태
		Failure = 2      // 로그인 실패 상태
	};
	// 로그인 상태를 나타내는 변수
	// 0: 로그인 이전 상태
	// 1: 로그인 성공 상태
	// 2: 로그인 실패 상태
	RegisterStatus m_registerStatus = RegisterStatus::None;

	std::string m_userId = "localuser";
	std::string m_userPw;
	std::string m_class_type;
};