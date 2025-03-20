#include "pch.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainData::TerrainData(std::wstring_view instancesPath, float terrainScale, float instanceScale) : m_terrainScale(terrainScale), m_instanceScale(instanceScale)
{
	CreateBuffer(instancesPath);
}

void TerrainData::CreateBuffer(std::wstring_view instancesPath)
{
	std::vector<std::pair<int, Matrix4x4>> intermediateData;
	std::vector<Matrix4x4> bufferData;

	std::ifstream file(instancesPath.data());
	nlohmann::json j;
	file >> j;

	for (auto& prototype : j)
	{
		std::string prototypeName = prototype["prefab"];
		for (auto& instance : prototype["instances"])
		{
			Vector3 position = Vector3(instance["position"]["x"] * -1.0f, instance["position"]["y"], instance["position"]["z"] * -1.0f);
			Quaternion rotation = Quaternion(instance["rotation"]["x"], instance["rotation"]["y"], instance["rotation"]["z"], instance["rotation"]["w"]);
			Vector3 scale = Vector3(instance["scale"]["x"], instance["scale"]["y"], instance["scale"]["z"]);
			Matrix4x4 instanceMatrix =
				Matrix4x4::CreateScale(scale * m_instanceScale) *
				Matrix4x4::CreateFromQuaternion(rotation) *
				Matrix4x4::CreateTranslation(position * m_terrainScale);
			m_instanceData[prototypeName].emplace_back(instanceMatrix.Transpose());
		}
	}
}

UINT TerrainData::GetPrototypeCount() const
{
	return static_cast<UINT>(m_instanceData.size());
}

UINT TerrainData::GetPrototypeInstanceCount(std::string_view prototypeName) const
{
	if (m_instanceData.find(prototypeName.data()) != m_instanceData.end())
	{
		return static_cast<UINT>(m_instanceData.at(prototypeName.data()).size());
	}
	return 0;
}

UINT TerrainData::PopulateInstanceData(std::string_view prototypeName, const udsdx::BoundingCamera& bound, const udsdx::Matrix4x4& worldTransform, const udsdx::Mesh* mesh, udsdx::Matrix4x4* out) const
{
	UINT instanceCount = 0;
	std::vector<Matrix4x4> instances = m_instanceData.at(prototypeName.data());
	UINT count = GetPrototypeInstanceCount(prototypeName);

	for (UINT index = 0; index < count; index++)
	{
		Matrix4x4 instance = instances[index];
		Matrix4x4 world = instance.Transpose() * worldTransform;
		BoundingBox boxWorld;
		mesh->GetBounds().Transform(boxWorld, world);

		if (bound.Contains(boxWorld) != ContainmentType::DISJOINT)
		{
			out[instanceCount++] = instance;
		}
	}
	return instanceCount;
}
