#include "pch.h"
#include "TerrainInstanceRenderer.h"
#include "TerrainData.h"

using namespace udsdx;

TerrainInstanceRenderer::TerrainInstanceRenderer(const std::shared_ptr<udsdx::SceneObject>& object) : RendererBase(object)
{
}

void TerrainInstanceRenderer::Render(udsdx::RenderParam& param, int instances)
{
	ObjectConstants objectConstants;
	objectConstants.World = m_transformCache.Transpose();
	objectConstants.PrevWorld = m_prevTransformCache.Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

	UINT numPrototype = m_terrainData->GetPrototypeCount();
	for (UINT prototypeIndex = 0; prototypeIndex < numPrototype; ++prototypeIndex)
	{
		param.CommandList->SetGraphicsRootDescriptorTable(RootParam::BonesSRV, m_terrainData->GetTransformGpuSrv(prototypeIndex));

		const auto& mesh = m_meshes[prototypeIndex];

		D3D12_VERTEX_BUFFER_VIEW vbv = mesh->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibv = mesh->IndexBufferView();

		param.CommandList->IASetVertexBuffers(0, 1, &vbv);
		param.CommandList->IASetIndexBuffer(&ibv);
		param.CommandList->IASetPrimitiveTopology(m_topology);

		UINT prototypeBase = m_terrainData->GetPrototypeInstanceBase(prototypeIndex);
		UINT prototypeCount = m_terrainData->GetPrototypeInstanceCount(prototypeIndex);

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
			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, prototypeCount * instances, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
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
	m_terrainData = terrainData;
}

ID3D12PipelineState* TerrainInstanceRenderer::GetPipelineState() const
{
	return m_shader->DefaultPipelineState();
}

ID3D12PipelineState* TerrainInstanceRenderer::GetShadowPipelineState() const
{
	return m_shader->ShadowPipelineState();
}