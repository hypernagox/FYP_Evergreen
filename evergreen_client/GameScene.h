#pragma once

#include "pch.h"

class HeightMap;
class TerrainData;
class TerrainDetail;
class AuthenticPlayer;
class InteractiveEntity;

class GameScene : public udsdx::Scene
{
public:
	GameScene(HeightMap* heightMap, TerrainData* terrainData, TerrainDetail* terrainDetail);

public:
	void Update(const udsdx::Time& time) override;
	// 플레이어를 생성하고 월드에 추가하는 단계
	void EnterGame();
	// 게임 종료
	void ExitGame();
	void OnTogglePause(bool isPaused);
	void OnTogglePlayerMode(bool spectatorMode);
	void AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	std::vector<InteractiveEntity*> GetInteractiveEntities() const;

	udsdx::Camera* GetMainCamera() const;

private:
	std::shared_ptr<udsdx::SceneObject> m_activeObjectGroup;

	std::vector<std::shared_ptr<udsdx::Material>> m_instanceMaterials;

	std::unique_ptr<SoundEffectInstance> m_menuSound;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuCameraObject;
	std::shared_ptr<udsdx::SceneObject> m_heroObj;
	std::shared_ptr<udsdx::SceneObject> m_spectatorObj;
	std::shared_ptr<udsdx::SceneObject> m_playerLightObj;
	std::shared_ptr<udsdx::SceneObject> m_terrainObj;

	std::shared_ptr<udsdx::SceneObject> m_playerInterfaceGroup;
	std::shared_ptr<udsdx::SceneObject> m_focusAgentObj;
	std::shared_ptr<udsdx::SceneObject> m_inventoryObj;
	std::shared_ptr<udsdx::SceneObject> m_craftObj;
	std::shared_ptr<udsdx::SceneObject> m_pauseMenuObj;
	std::shared_ptr<udsdx::SceneObject> m_partyListObj;
	std::shared_ptr<udsdx::SceneObject> m_playerTagObj;

	AuthenticPlayer* m_heroComponent;

	std::shared_ptr<udsdx::Material> m_terrainMaterial;
	std::shared_ptr<udsdx::Material> m_terrainDetailMaterial;
	std::shared_ptr<udsdx::Material> m_playerMaterial;
	std::shared_ptr<udsdx::Material> m_skyboxMaterial;
	std::shared_ptr<udsdx::Material> m_gizmoMaterial;
	std::shared_ptr<udsdx::Mesh> m_terrainMesh;

	bool m_bSpectatorMode = false;
};

