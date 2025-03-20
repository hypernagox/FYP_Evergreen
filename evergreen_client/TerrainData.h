#pragma once

#include "pch.h"

class TerrainData
{
public:
	TerrainData(std::wstring_view instancesPath, float terrainScale, float instanceScale);
	void CreateBuffer(std::wstring_view instancesPath);

	UINT GetPrototypeCount() const;
	UINT GetPrototypeInstanceCount(std::string_view prototypeName) const;
	UINT PopulateInstanceData(std::string_view prototypeName, const udsdx::BoundingCamera& bound, const udsdx::Matrix4x4& worldTransform, const udsdx::Mesh* mesh, udsdx::Matrix4x4* out) const;

private:
	float m_terrainScale = 1.0f;
	float m_instanceScale = 1.0f;

	std::unordered_map<std::string, std::vector<udsdx::Matrix4x4>> m_instanceData;
};