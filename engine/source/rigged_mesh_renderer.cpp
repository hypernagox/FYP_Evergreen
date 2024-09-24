#include "pch.h"
#include "rigged_mesh_renderer.h"
#include "renderer_base.h"
#include "frame_resource.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"
#include "rigged_mesh.h"
#include "core.h"

namespace udsdx
{
	RiggedMeshRenderer::RiggedMeshRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
	{
		for (auto& buffer : m_constantBuffers)
		{
			buffer = std::make_unique<UploadBuffer<BoneConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
		}
	}

	void RiggedMeshRenderer::Update(const Time& time, Scene& scene)
	{
		m_animationTime += time.deltaTime;
		m_isMatrixDirty = true;

		RendererBase::Update(time, scene);
	}

	void RiggedMeshRenderer::Render(RenderParam& param, int instances)
	{
		Transform* transform = GetSceneObject()->GetTransform();
		Matrix4x4 worldMat = transform->GetWorldSRTMatrix();

		if (param.UseFrustumCulling)
		{
			// Perform frustum culling
			BoundingBox boundsWorld;
			m_riggedMesh->GetBounds().Transform(boundsWorld, worldMat);
			if (param.ViewFrustumWorld.Contains(boundsWorld) == ContainmentType::DISJOINT)
			{
				return;
			}
		}

		// Update bone constants
		auto& uploader = m_constantBuffers[param.FrameResourceIndex];
		if (m_isMatrixDirty)
		{
			BoneConstants boneConstants{};
			std::vector<Matrix4x4> boneTransforms;
			if (!m_animationName.empty())
			{
				m_riggedMesh->PopulateTransforms(m_animationName, m_animationTime, boneTransforms);
			}
			else
			{
				m_riggedMesh->PopulateTransforms(boneTransforms);
			}
			for (size_t i = 0; i < boneTransforms.size(); ++i)
			{
				boneConstants.BoneTransforms[i] = boneTransforms[i].Transpose();
			}
			uploader->CopyData(0, boneConstants);
			m_isMatrixDirty = false;
		}

		ObjectConstants objectConstants;
		objectConstants.World = worldMat.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
		param.CommandList->SetGraphicsRootConstantBufferView(RootParam::BonesCBV, uploader->Resource()->GetGPUVirtualAddress());

		param.CommandList->IASetVertexBuffers(0, 1, &m_riggedMesh->VertexBufferView());
		param.CommandList->IASetIndexBuffer(&m_riggedMesh->IndexBufferView());

		const auto& submeshes = m_riggedMesh->GetSubmeshes();
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

	void RiggedMeshRenderer::SetMesh(RiggedMesh* mesh)
	{
		m_riggedMesh = mesh;
		m_isMatrixDirty = true;
	}

	void RiggedMeshRenderer::SetAnimation(std::string_view animationName)
	{
		m_animationTime = 0.0f;
		m_animationName = animationName;
		m_isMatrixDirty = true;
	}

	ID3D12PipelineState* RiggedMeshRenderer::GetPipelineState() const
	{
		return m_shader->RiggedPipelineState();
	}

	ID3D12PipelineState* RiggedMeshRenderer::GetShadowPipelineState() const
	{
		return m_shader->RiggedShadowPipelineState();
	}
}