#pragma once

#include "pch.h"

class HeightMap;
class TerrainData;
class TerrainDetail;

struct EnvironmentParameters
{
public:
	HeightMap* HeightMap;
	TerrainData* TerrainData;
	TerrainDetail* TerrainDetail;
	float TerrainSize;
};

class EnvironmentRenderer : public udsdx::Component
{
public:
	EnvironmentRenderer(const std::shared_ptr<udsdx::SceneObject>& object);
	void Initialize(const EnvironmentParameters& parameters);

public:
	std::shared_ptr<udsdx::SceneObject> AddTerrainInstances(std::filesystem::path path, TerrainData* terrainData);
	std::shared_ptr<udsdx::SceneObject> AddHarvestObject(const nlohmann::json& instance);
	udsdx::Mesh* GetTerrainMesh() const { return m_terrainMesh.get(); }

private:
	std::unordered_map<std::string, udsdx::Texture*> m_textureMap;
	std::unordered_map<std::string, std::filesystem::path> m_prefabMap;

	std::vector<std::shared_ptr<udsdx::Material>> m_instanceMaterials;

	std::shared_ptr<udsdx::Material> m_terrainMaterial;
	std::shared_ptr<udsdx::Material> m_terrainDetailMaterial;
	std::shared_ptr<udsdx::Material> m_skyboxMaterial;

	std::shared_ptr<udsdx::Mesh> m_terrainMesh;

	std::shared_ptr<udsdx::SceneObject> m_terrainObj;
};