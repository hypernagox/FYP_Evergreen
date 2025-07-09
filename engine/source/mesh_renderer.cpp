#include "pch.h"
#include "frame_resource.h"
#include "mesh_renderer.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "scene.h"
#include "mesh.h"

namespace udsdx
{
	void MeshRenderer::Render(RenderParam& param, int instances)
	{
		if (param.UseFrustumCulling)
		{
			// Perform frustum culling
			BoundingBox boundsWorld;
			m_mesh->GetBounds().Transform(boundsWorld, m_transformCache);
			if (param.ViewFrustumWorld->Contains(boundsWorld) == ContainmentType::DISJOINT)
			{
				return;
			}
		}

		ObjectConstants objectConstants;
		objectConstants.World = m_transformCache.Transpose();
		objectConstants.PrevWorld = m_prevTransformCache.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

		param.CommandList->IASetVertexBuffers(0, 1, &m_mesh->VertexBufferView());
		param.CommandList->IASetIndexBuffer(&m_mesh->IndexBufferView());
		param.CommandList->IASetPrimitiveTopology(m_topology);

		const auto& submeshes = m_mesh->GetSubmeshes();
		for (size_t index = 0; index < submeshes.size(); ++index)
		{
			if (index < m_materials.size() && m_materials[index] != nullptr)
			{
				for (UINT textureSrcIndex = 0; textureSrcIndex < m_materials[index]->GetTextureCount(); ++textureSrcIndex)
				{
					const Texture* texture = m_materials[index]->GetSourceTexture(textureSrcIndex);
					if (texture != nullptr)
					{
						param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0 + textureSrcIndex, texture->GetSrvGpu());
					}
				}
			}
			const auto& submesh = submeshes[index];
			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instances, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
		}
	}

	void MeshRenderer::OnDrawGizmos(const Camera* target)
	{
		if (m_mesh == nullptr)
		{
			return;
		}

		BoundingBox boundsWorld;
		m_mesh->GetBounds().Transform(boundsWorld, m_transformCache);
		
		std::array<Vector3, BoundingBox::CORNER_COUNT> corners;
		std::array<Vector2, BoundingBox::CORNER_COUNT> cornersScreen;

		boundsWorld.GetCorners(corners.data());

		bool isVisible = true;
		for (size_t i = 0; i < BoundingBox::CORNER_COUNT && isVisible; ++i)
		{
			isVisible &= target->ToViewPosition(corners[i]).z > 1e-2f;
			cornersScreen[i] = target->ToScreenPosition(corners[i]);
		}

		if (!isVisible)
		{
			return;
		}

		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		ImColor drawColor(1.0f, 1.0f, 1.0f, 1.0f);
		int indices[] = {
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};
		for (size_t i = 0; i < 12; ++i)
		{
			int start = indices[i << 1];
			int end = indices[i << 1 | 1];
			drawList->AddLine(
				ImVec2(cornersScreen[start].x, cornersScreen[start].y),
				ImVec2(cornersScreen[end].x, cornersScreen[end].y),
				drawColor);
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