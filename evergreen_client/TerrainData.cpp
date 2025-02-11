#include "pch.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainData::TerrainData(std::wstring_view instancesPath)
{
	CreateBuffer(instancesPath);
	UploadBuffer(INSTANCE(Core)->GetDevice(), INSTANCE(Core)->GetCommandList());
	BuildDescriptors();
}

void TerrainData::CreateBuffer(std::wstring_view instancesPath)
{
	std::vector<std::pair<int, Matrix4x4>> intermediateData;
	std::vector<std::vector<Matrix4x4>> bufferData;

	std::ifstream file(instancesPath.data());
	nlohmann::json j;
	file >> j;

	for (auto& tree : j)
	{
		Vector3 position = Vector3(tree["position_x"], tree["position_y"], tree["position_z"]);
		Quaternion rotation = Quaternion::CreateFromYawPitchRoll(0.0f, -PIDIV2, tree["rotation_y"]);
		Vector3 scale = Vector3(tree["size_width"], tree["size_height"], tree["size_width"]);
		int treePrototype = tree["prototype_index"];

		Matrix4x4 treeMatrix = Matrix4x4::CreateScale(scale * 0.02f) *
			Matrix4x4::CreateFromQuaternion(rotation) *
			Matrix4x4::CreateTranslation(position * Vector3(-1.0f, 1.0f, -1.0f) * 512.0f + Vector3::Up * -100.0f);
		intermediateData.emplace_back(treePrototype, treeMatrix.Transpose());
	}

	std::sort(intermediateData.begin(), intermediateData.end(), [](const std::pair<int, Matrix4x4>& a, const std::pair<int, Matrix4x4>& b) { return a.first < b.first; });
	int base = 0, p = 0;
	bufferData.emplace_back();
	for (size_t i = 0; i < intermediateData.size(); i++)
	{
		if (i > 0 && intermediateData[i].first != intermediateData[i - 1].first)
		{
			m_baseCountPairs.emplace_back(base, i - base);
			base = i;
			bufferData.emplace_back();
			++p;
		}
		bufferData[p].emplace_back(std::move(intermediateData[i].second));
	}
	m_baseCountPairs.emplace_back(base, intermediateData.size() - base);

	m_prototypeCount = static_cast<UINT>(bufferData.size());
	m_bufferCPU.resize(m_prototypeCount);
	m_bufferGpu.resize(m_prototypeCount);
	m_bufferUpload.resize(m_prototypeCount);

	m_desciptorCpuSrv.resize(m_prototypeCount);
	m_desciptorGpuSrv.resize(m_prototypeCount);

	for (UINT i = 0; i < m_prototypeCount; ++i)
	{
		const UINT bufferByteSize = static_cast<UINT>(bufferData[i].size()) * sizeof(Matrix4x4);
		m_bufferCPU[i] = ComPtr<ID3DBlob>();

		ThrowIfFailed(D3DCreateBlob(bufferByteSize, &m_bufferCPU[i]));
		CopyMemory(m_bufferCPU[i]->GetBufferPointer(), bufferData[i].data(), bufferByteSize);
	}
}

void TerrainData::UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{	
	for (UINT i = 0; i < m_prototypeCount; ++i)
	{
		// Make sure buffers are uploaded to the CPU.
		assert(m_bufferCPU[i] != nullptr);

		m_bufferGpu[i] = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_bufferCPU[i]->GetBufferPointer(),
			m_bufferCPU[i]->GetBufferSize(),
			m_bufferUpload[i]
		);
	}
}

void TerrainData::BuildDescriptors()
{
	for (UINT i = 0; i < m_prototypeCount; ++i)
	{
		INSTANCE(Core)->IncrementSRVHeapDescriptor(&m_desciptorCpuSrv[i], &m_desciptorGpuSrv[i]);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_bufferCPU[i]->GetBufferSize() / sizeof(Matrix4x4);
		srvDesc.Buffer.StructureByteStride = sizeof(Matrix4x4);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		INSTANCE(Core)->GetDevice()->CreateShaderResourceView(m_bufferGpu[i].Get(), &srvDesc, m_desciptorCpuSrv[i]);
	}
}

CD3DX12_GPU_DESCRIPTOR_HANDLE TerrainData::GetTransformGpuSrv(int index) const
{
	return m_desciptorGpuSrv[index];
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

