#include "pch.h"
#include "frame_resource.h"
#include "mesh_renderer.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"
#include "mesh.h"

namespace udsdx
{
	MeshRenderer::MeshRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
	{
	}

	void MeshRenderer::Render(RenderParam& param, int instances)
	{
		Transform* transform = GetSceneObject()->GetTransform();
		Matrix4x4 worldMat = transform->GetWorldSRTMatrix();

		if (param.UseFrustumCulling)
		{
			// Perform frustum culling
			BoundingBox boundsWorld;
			m_mesh->GetBounds().Transform(boundsWorld, worldMat);
			if (param.ViewFrustumWorld.Contains(boundsWorld) == ContainmentType::DISJOINT)
			{
				return;
			}
		}

		ObjectConstants objectConstants;
		objectConstants.World = worldMat.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

		param.CommandList->IASetVertexBuffers(0, 1, &m_mesh->VertexBufferView());
		param.CommandList->IASetIndexBuffer(&m_mesh->IndexBufferView());

		const auto& submeshes = m_mesh->GetSubmeshes();
		for (size_t index = 0; index < submeshes.size(); ++index)
		{
			if (index < m_materials.size() && m_materials[index] != nullptr)
			{
				Texture* mainTex = m_materials[index]->GetMainTexture();
				if (mainTex != nullptr)
				{
					param.CommandList->SetGraphicsRootDescriptorTable(RootParam::MainTexSRV, mainTex->GetSrvGpu());
				}
				Texture* normalTex = m_materials[index]->GetNormalTexture();
				if (normalTex != nullptr)
				{
					param.CommandList->SetGraphicsRootDescriptorTable(RootParam::NormalSRV, normalTex->GetSrvGpu());
				}
			}
			const auto& submesh = submeshes[index];
			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instances, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
		}
	}

	void MeshRenderer::SetMesh(Mesh* mesh)
	{
		m_mesh = mesh;
	}

	Mesh* MeshRenderer::GetMesh() const
	{
		return m_mesh;
	}

	ID3D12PipelineState* MeshRenderer::GetPipelineState() const
	{
		return m_shader->DefaultPipelineState();
	}

	ID3D12PipelineState* MeshRenderer::GetShadowPipelineState() const
	{
		return m_shader->ShadowPipelineState();
	}
}