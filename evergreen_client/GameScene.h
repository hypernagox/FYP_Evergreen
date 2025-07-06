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
	enum class GameSceneType
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
	void EnterGame(std::shared_ptr<GameScene> sharedScene, unsigned int character, int channelID);
	// 게임 종료
	void ExitGame();
	void OnTogglePause(bool isPaused);
	void OnTogglePlayerMode(bool spectatorMode);
	void AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	void AddInterfaceObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	bool GetSpectatorMode() const { return m_bSpectatorMode; }
	void RequestChangeGameScene(GameSceneType type);
	void ChangeGameScene(GameSceneType type);
	std::vector<InteractiveEntity*> GetInteractiveEntities() const;

	udsdx::Camera* GetMainCamera() const;

private:
	GameSceneType m_sceneType = GameSceneType::Default;
	std::shared_ptr<udsdx::SceneObject> m_activeObjectGroup;

	std::vector<std::shared_ptr<udsdx::Material>> m_instanceMaterials;
	std::vector<std::shared_ptr<udsdx::Material>> m_harvestMaterials;

	std::shared_ptr<udsdx::SceneObject> m_defaultEnvironmentObject;
	std::shared_ptr<udsdx::SceneObject> m_dungeonEnvironmentObject;

	std::shared_ptr<udsdx::SceneObject> m_heroObj;
	std::shared_ptr<udsdx::SceneObject> m_spectatorObj;
	std::shared_ptr<udsdx::SceneObject> m_environmentLightObj;
	std::shared_ptr<udsdx::SceneObject> m_craftTableObj;

	std::shared_ptr<udsdx::SceneObject> m_interfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_playerInterfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_focusAgentObj;
	std::shared_ptr<udsdx::SceneObject> m_inventoryObj;
	std::shared_ptr<udsdx::SceneObject> m_equipmentObj;
	std::shared_ptr<udsdx::SceneObject> m_tutorialObj;
	std::shared_ptr<udsdx::SceneObject> m_craftObj;
	std::shared_ptr<udsdx::SceneObject> m_pauseMenuObj;
	std::shared_ptr<udsdx::SceneObject> m_partyListObj;
	std::shared_ptr<udsdx::SceneObject> m_playerTagObj;
	std::shared_ptr<udsdx::SceneObject> m_channelSwitchObj;

	AuthenticPlayer* m_heroComponent;
	ServerObject* m_heroServerObject;
	PopupGUIManager* m_popupGUIManager;

	std::shared_ptr<udsdx::Material> m_playerMaterial;
	std::shared_ptr<udsdx::Material> m_skyboxMaterial;
	std::shared_ptr<udsdx::Material> m_gizmoMaterial;
	std::shared_ptr<udsdx::Material> m_craftTableMaterial;

	bool m_bSpectatorMode = false;

	std::unique_ptr<MinimapRenderer> m_minimapRenderer;

	int m_currentChannelID = 0;
};