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
	// �÷��̾ �����ϰ� ���忡 �߰��ϴ� �ܰ�
	void EnterGame();
	// ���� ����
	void ExitGame();
	void OnTogglePause(bool isPaused);
	void OnTogglePlayerMode(bool spectatorMode);
	void AddActiveObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	void AddInterfaceObject(const std::shared_ptr<udsdx::SceneObject>& obj);
	bool GetSpectatorMode() const { return m_bSpectatorMode; }
	std::vector<InteractiveEntity*> GetInteractiveEntities() const;

	udsdx::Camera* GetMainCamera() const;

private:
	void AddTerrainInstances(std::filesystem::path path, const std::map<std::string, udsdx::Texture*>& textureMap, TerrainData* terrainData);
	void AddHarvestObjects(std::filesystem::path path, const std::map<std::string, udsdx::Texture*>& textureMap, const nlohmann::json& prototype);

private:
	std::shared_ptr<udsdx::SceneObject> m_activeObjectGroup;

	std::vector<std::shared_ptr<udsdx::Material>> m_instanceMaterials;
	std::vector<std::shared_ptr<udsdx::Material>> m_harvestMaterials;

	std::unique_ptr<SoundEffectInstance> m_menuSound;
	std::shared_ptr<udsdx::SceneObject> m_mainMenuCameraObject;
	std::shared_ptr<udsdx::SceneObject> m_heroObj;
	std::shared_ptr<udsdx::SceneObject> m_spectatorObj;
	std::shared_ptr<udsdx::SceneObject> m_playerLightObj;
	std::shared_ptr<udsdx::SceneObject> m_terrainObj;
	std::shared_ptr<udsdx::SceneObject> m_craftTableObj;

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
	std::shared_ptr<udsdx::Material> m_craftTableMaterial;

	std::shared_ptr<udsdx::Mesh> m_terrainMesh;

	bool m_bSpectatorMode = false;
};