#include "pch.h"
#include "TerrainInstanceRenderer.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainInstanceRenderer::TerrainInstanceRenderer(const std::shared_ptr<udsdx::SceneObject>& object) : RendererBase(object)
{
}

void TerrainInstanceRenderer::Render(udsdx::RenderParam& param, int instances)
{
	if (param.RenderStageIndex == 3)
	{
		return;
	}

	static std::vector<udsdx::Matrix4x4> instanceBuffer;
	UINT prototypeCount = m_terrainData->GetPrototypeInstanceCount(m_prototypeName);
	instanceBuffer.resize(prototypeCount);

	ObjectConstants objectConstants;
	objectConstants.World = m_transformCache.Transpose();
	objectConstants.PrevWorld = m_prevTransformCache.Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

	// Resize instance upload buffer if necessary
	while (m_instanceUploadBuffer.size() <= static_cast<size_t>(param.RenderStageIndex))
	{
		auto& uploadBuffer = m_instanceUploadBuffer.emplace_back();
		for (auto& subBuffer : uploadBuffer)
		{
			subBuffer = std::make_unique<UploadBuffer<udsdx::Matrix4x4>>(INSTANCE(Core)->GetDevice(), prototypeCount, false);
		}
	}

	UINT instanceCount = m_terrainData->PopulateInstanceData(m_prototypeName, *param.ViewFrustumWorld, m_transformCache, m_mesh, instanceBuffer.data());

	if (instanceCount == 0)
	{
		return;
	}

	auto* buffer = m_instanceUploadBuffer[param.RenderStageIndex][param.FrameResourceIndex].get();
	buffer->CopyData(0, instanceBuffer);

	D3D12_VERTEX_BUFFER_VIEW vibv;
	vibv.BufferLocation = buffer->Resource()->GetGPUVirtualAddress();
	vibv.StrideInBytes = sizeof(udsdx::Matrix4x4);
	vibv.SizeInBytes = instanceCount * sizeof(udsdx::Matrix4x4);

	D3D12_VERTEX_BUFFER_VIEW vbv = m_mesh->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW ibv = m_mesh->IndexBufferView();

	param.CommandList->IASetVertexBuffers(0, 1, &vbv);
	param.CommandList->IASetVertexBuffers(1, 1, &vibv);
	param.CommandList->IASetIndexBuffer(&ibv);
	param.CommandList->IASetPrimitiveTopology(m_topology);

	const auto& submeshes = m_mesh->GetSubmeshes();
	for (size_t index = 0; index < submeshes.size(); ++index)
	{
		if (index < m_materials.size() && m_materials[index] != nullptr)
		{
			for (UINT textureSrcIndex = 0; textureSrcIndex < m_materials[index]->GetTextureCount(); ++textureSrcIndex)
			{
				const udsdx::Texture* texture = m_materials[index]->GetSourceTexture(textureSrcIndex);
				if (texture != nullptr)
				{
					param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0 + textureSrcIndex, texture->GetSrvGpu());
				}
			}
		}
		const auto& submesh = submeshes[index];
		param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instanceCount, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
	}
}

void TerrainInstanceRenderer::SetMesh(udsdx::Mesh* mesh)
{
	m_mesh = mesh;
}

TerrainData* TerrainInstanceRenderer::GetTerrainData() const
{
	return m_terrainData;
}

void TerrainInstanceRenderer::SetTerrainData(TerrainData* terrainData, std::string_view prototypeName)
{
	if (m_terrainData == terrainData && m_prototypeName == prototypeName)
	{
		return;
	}
	m_terrainData = terrainData;
	m_prototypeName = prototypeName;

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