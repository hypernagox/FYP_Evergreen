#include "pch.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainData::TerrainData(std::wstring_view instancesPath, float terrainScale, float instanceScale) : m_terrainScale(terrainScale), m_instanceScale(instanceScale)
{
	CreateBuffer(instancesPath);
	UploadBuffer(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());
}

void TerrainData::CreateBuffer(std::wstring_view instancesPath)
{
	std::vector<std::pair<int, Matrix4x4>> intermediateData;
	std::vector<Matrix4x4> bufferData;

	std::ifstream file(instancesPath.data());
	nlohmann::json j;
	file >> j;

	for (auto& tree : j)
	{
		Vector3 position = Vector3(tree["position_x"], tree["position_y"], tree["position_z"]);
		Quaternion rotation = Quaternion::CreateFromYawPitchRoll(0.0f, -PIDIV2, tree["rotation_y"]);
		Vector3 scale = Vector3(tree["size_width"], tree["size_height"], tree["size_width"]);
		int treePrototype = tree["prototype_index"];

		Matrix4x4 treeMatrix = Matrix4x4::CreateScale(scale * m_instanceScale) *
			Matrix4x4::CreateFromQuaternion(rotation) *
			Matrix4x4::CreateTranslation(position * Vector3(-1.0f, 1.0f, -1.0f) * m_terrainScale);
		intermediateData.emplace_back(treePrototype, treeMatrix.Transpose());
	}

	std::sort(intermediateData.begin(), intermediateData.end(), [](const std::pair<int, Matrix4x4>& a, const std::pair<int, Matrix4x4>& b) { return a.first < b.first; });
	int base = 0;
	for (size_t i = 0; i < intermediateData.size(); i++)
	{
		if (i > 0 && intermediateData[i].first != intermediateData[i - 1].first)
		{
			m_baseCountPairs.emplace_back(static_cast<UINT>(base), static_cast<UINT>(i - base));
			base = static_cast<UINT>(i);
		}
		bufferData.emplace_back(intermediateData[i].second);
	}
	m_baseCountPairs.emplace_back(static_cast<UINT>(base), static_cast<UINT>(intermediateData.size() - base));

	m_prototypeCount = static_cast<UINT>(bufferData.size());
	m_vertexBufferByteSize = static_cast<UINT>(bufferData.size()) * sizeof(Matrix4x4);

	ThrowIfFailed(D3DCreateBlob(m_vertexBufferByteSize, &m_bufferCPU));
	CopyMemory(m_bufferCPU->GetBufferPointer(), bufferData.data(), m_vertexBufferByteSize);
}

void TerrainData::UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{	
	assert(m_bufferCPU != nullptr);

	m_bufferGpu = d3dUtil::CreateDefaultBuffer(
		device,
		commandList,
		m_bufferCPU->GetBufferPointer(),
		m_bufferCPU->GetBufferSize(),
		m_bufferUpload
	);
}

D3D12_VERTEX_BUFFER_VIEW TerrainData::GetTransformBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_bufferGpu->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_vertexByteStride;
	vbv.SizeInBytes = m_vertexBufferByteSize;

	return vbv;
}

UINT TerrainData::GetPrototypeCount() const
{
	return static_cast<UINT>(m_baseCountPairs.size());
}

UINT TerrainData::GetPrototypeInstanceBase(int index) const
{
	return m_baseCountPairs[index].first;
}

UINT TerrainData::GetPrototypeInstanceCount(int index) const
{
	return m_baseCountPairs[index].second;
}

UINT TerrainData::PopulateInstanceData(UINT prototypeIndex, const udsdx::BoundingCamera& bound, const udsdx::Matrix4x4& worldTransform, const udsdx::Mesh* mesh, udsdx::Matrix4x4* out) const
{
	UINT instanceCount = 0;
	UINT base = GetPrototypeInstanceBase(prototypeIndex);
	UINT count = GetPrototypeInstanceCount(prototypeIndex);

	for (UINT index = 0; index < count; index++)
	{
		Matrix4x4 instance = reinterpret_cast<Matrix4x4*>(m_bufferCPU->GetBufferPointer())[base + index];
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