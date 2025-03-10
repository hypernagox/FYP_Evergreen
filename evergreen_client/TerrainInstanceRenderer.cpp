#include "pch.h"
#include "TerrainInstanceRenderer.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainInstanceRenderer::TerrainInstanceRenderer(const std::shared_ptr<udsdx::SceneObject>& object) : RendererBase(object)
{
}

void TerrainInstanceRenderer::Render(udsdx::RenderParam& param, int instances)
{
	static std::vector<udsdx::Matrix4x4> instanceBuffer;

	ObjectConstants objectConstants;
	objectConstants.World = m_transformCache.Transpose();
	objectConstants.PrevWorld = m_prevTransformCache.Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

	UINT numPrototype = m_terrainData->GetPrototypeCount();
	UINT instanceCount = m_terrainData->GetPrototypeInstanceBase(numPrototype - 1) + m_terrainData->GetPrototypeInstanceCount(numPrototype - 1);

	// Resize instance upload buffer if necessary
	while (m_instanceUploadBuffer.size() <= static_cast<size_t>(param.RenderStageIndex))
	{
		auto& uploadBuffer = m_instanceUploadBuffer.emplace_back();
		for (auto& subBuffer : uploadBuffer)
		{
			subBuffer = std::make_unique<UploadBuffer<udsdx::Matrix4x4>>(INSTANCE(Core)->GetDevice(), instanceCount, false);
		}
	}
	auto* buffer = m_instanceUploadBuffer[param.RenderStageIndex][param.FrameResourceIndex].get();

	D3D12_VERTEX_BUFFER_VIEW vibv;
	vibv.BufferLocation = buffer->Resource()->GetGPUVirtualAddress();
	vibv.StrideInBytes = sizeof(udsdx::Matrix4x4);
	vibv.SizeInBytes = instanceCount * sizeof(udsdx::Matrix4x4);

	for (UINT prototypeIndex = 0; prototypeIndex < numPrototype; ++prototypeIndex)
	{
		UINT prototypeBase = m_terrainData->GetPrototypeInstanceBase(prototypeIndex);
		UINT prototypeCount = m_terrainData->GetPrototypeInstanceCount(prototypeIndex);

		instanceBuffer.resize(prototypeCount);
		UINT instanceCount = m_terrainData->PopulateInstanceData(prototypeIndex, *param.ViewFrustumWorld, m_transformCache, m_meshes[prototypeIndex], instanceBuffer.data());
		buffer->CopyData(prototypeBase, instanceBuffer);

		if (instanceCount == 0)
		{
			continue;
		}

		const auto& mesh = m_meshes[prototypeIndex];

		D3D12_VERTEX_BUFFER_VIEW vbv = mesh->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibv = mesh->IndexBufferView();

		param.CommandList->IASetVertexBuffers(0, 1, &vbv);
		param.CommandList->IASetVertexBuffers(1, 1, &vibv);
		param.CommandList->IASetIndexBuffer(&ibv);
		param.CommandList->IASetPrimitiveTopology(m_topology);

		const auto& submeshes = mesh->GetSubmeshes();
		for (size_t index = 0; index < submeshes.size(); ++index)
		{
			if (index < m_materials.size() && m_materials[index] != nullptr)
			{
				udsdx::Texture* mainTex = m_materials[index]->GetMainTexture();
				if (mainTex != nullptr)
				{
					param.CommandList->SetGraphicsRootDescriptorTable(RootParam::MainTexSRV, mainTex->GetSrvGpu());
				}
				udsdx::Texture* normalTex = m_materials[index]->GetNormalTexture();
				if (normalTex != nullptr)
				{
					param.CommandList->SetGraphicsRootDescriptorTable(RootParam::NormalSRV, normalTex->GetSrvGpu());
				}
			}
			const auto& submesh = submeshes[index];
			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instanceCount, submesh.StartIndexLocation, submesh.BaseVertexLocation, prototypeBase);
		}
	}
}

void TerrainInstanceRenderer::AddMesh(udsdx::Mesh* mesh)
{
	m_meshes.push_back(mesh);
}

TerrainData* TerrainInstanceRenderer::GetTerrainData() const
{
	return m_terrainData;
}

void TerrainInstanceRenderer::SetTerrainData(TerrainData* terrainData)
{
	if (m_terrainData == terrainData)
	{
		return;
	}
	m_terrainData = terrainData;

	INSTANCE(Core)->FlushCommandQueue();
	m_instanceUploadBuffer.clear();
}

ID3D12PipelineState* TerrainInstanceRenderer::GetPipelineState() const
{
	return m_shader->DefaultPipelineState();
}

ID3D12PipelineState* TerrainInstanceRenderer::GetShadowPipelineState() const
{
	return m_shader->ShadowPipelineState();
}