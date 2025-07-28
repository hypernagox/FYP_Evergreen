#pragma once

#include "pch.h"
#include "MinimapRenderer.h"

class AuthenticPlayer;
class InteractiveEntity;
class PopupGUIManager;
class ServerObject;

class GameScene : public udsdx::Scene
{
public:
	enum class GameSceneType : std::uint8_t
	{
		Default,
		Dungeon
	};

public:
	GameScene();

public:
	void OnAttach() override;
	void OnDetach() override;
	void Update(const udsdx::Time& time) override;
	void Render(udsdx::RenderParam& param) override;
	// 플레이어를 생성하고 월드에 추가하는 단계
	void EnterGame(std::shared_ptr<GameScene> sharedScene, unsigned int character, int channelID, std::string_view username);
	// 게임 종료
	void ExitGame();
	void OnTogglePause(bool isPaused);
	void OnTogglePlayerMode(bool spectatorMode);
	void AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	void AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj, GameSceneType type);
	void AddInterfaceObject(const std::shared_ptr<udsdx::SceneObject>& obj, bool front = false);
	bool GetSpectatorMode() const { return m_bSpectatorMode; }
	std::shared_ptr<udsdx::SceneObject> GetHeroObject() const { return m_heroObj; }
	void RequestChangeGameScene(GameSceneType type);
	void ChangeGameScene(GameSceneType type);
	void AddMinimapMark(const Vector3& position, int index);
	void OnQuestEnd();
	void PlayMusic(std::wstring_view resourcePath);
	void SetBossMonsterCinematic(const std::shared_ptr<udsdx::SceneObject>& bossMonsterObj);

	void ShowDialogue(const std::shared_ptr<udsdx::SceneObject>& target, std::string_view dialogueKey, std::function<void()> endDialogueCallback = nullptr);
	void OnDialogueEnd();

	std::vector<InteractiveEntity*> GetInteractiveEntities() const;

	udsdx::Camera* GetMainCamera() const;

private:
	GameSceneType m_sceneType = GameSceneType::Default;
	std::shared_ptr<udsdx::SceneObject> m_activeObjectGroup;
	std::array<std::shared_ptr<udsdx::SceneObject>, 2> m_activeObjectSubGroups;

	std::shared_ptr<udsdx::SceneObject> m_defaultEnvironmentObject;
	std::shared_ptr<udsdx::SceneObject> m_dungeonEnvironmentObject;

	std::shared_ptr<udsdx::SceneObject> m_heroObj;
	std::shared_ptr<udsdx::SceneObject> m_spectatorObj;
	std::shared_ptr<udsdx::SceneObject> m_environmentLightObj;
	std::shared_ptr<udsdx::SceneObject> m_craftTableObj;
	std::shared_ptr<udsdx::SceneObject> m_jobBoardObj;
	std::shared_ptr<udsdx::SceneObject> m_npcObj;

	std::shared_ptr<udsdx::SceneObject> m_interfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_playerInterfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_playerInterfaceBackGroup;
	std::shared_ptr<udsdx::SceneObject> m_focusAgentObj;
	std::shared_ptr<udsdx::SceneObject> m_inventoryObj;
	std::shared_ptr<udsdx::SceneObject> m_equipmentObj;
	std::shared_ptr<udsdx::SceneObject> m_tutorialObj;
	std::shared_ptr<udsdx::SceneObject> m_craftObj;
	std::shared_ptr<udsdx::SceneObject> m_dialogueGUIObj;
	std::shared_ptr<udsdx::SceneObject> m_pauseMenuObj;
	std::shared_ptr<udsdx::SceneObject> m_partyListObj;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchObj;

	AuthenticPlayer* m_heroComponent;
	ServerObject* m_heroServerObject;
	PopupGUIManager* m_popupGUIManager;

	bool m_bSpectatorMode = false;

	std::unique_ptr<MinimapRenderer> m_minimapRenderer;
	std::vector<std::pair<Vector3, int>> m_minimapMarks;
	std::unique_ptr<SoundEffectInstance> m_ambienceSound;
	std::unique_ptr<SoundEffectInstance> m_bgmSound;

	int m_currentChannelID = 0;
};