#pragma once

#include "pch.h"

class HeightMap;
class TerrainData;
class TerrainDetail;
class AuthenticPlayer;

class GameScene : public udsdx::Scene
{
public:
	GameScene(HeightMap* heightMap, TerrainData* terrainData, TerrainDetail* terrainDetail);

public:
	void Update(const udsdx::Time& time) override;

private:
	std::vector<std::shared_ptr<udsdx::Material>> g_instanceMaterials;

	std::unique_ptr<SoundEffectInstance> g_menuSound;

	std::shared_ptr<udsdx::SceneObject> g_heroObj;
	std::shared_ptr<udsdx::SceneObject> playerLightObj;
	std::shared_ptr<udsdx::SceneObject> terrainObj;
	std::shared_ptr<udsdx::SceneObject> g_inventoryObj;
	std::shared_ptr<udsdx::SceneObject> g_craftObj;

	AuthenticPlayer* g_heroComponent;

	std::shared_ptr<udsdx::Material> terrainMaterial;
	std::shared_ptr<udsdx::Material> terrainDetailMaterial;
	std::shared_ptr<udsdx::Material> playerMaterial;
	std::shared_ptr<udsdx::Material> g_skyboxMaterial;
	std::shared_ptr<udsdx::Material> g_gizmoMaterial;
	std::shared_ptr<udsdx::Mesh> terrainMesh;
};

