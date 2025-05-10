#include "pch.h"
#include "rigged_mesh_renderer.h"
#include "animation_clip.h"
#include "renderer_base.h"
#include "frame_resource.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"
#include "rigged_mesh.h"
#include "camera.h"
#include "core.h"

namespace udsdx
{
	RiggedMeshRenderer::RiggedMeshRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
	{

	}

	void RiggedMeshRenderer::Update(const Time& time, Scene& scene)
	{
		m_animationTime += time.deltaTime;
		m_prevAnimationTime += time.deltaTime;
		m_transitionFactor += time.deltaTime / 0.2f;
		m_constantBuffersDirty = true;

		RendererBase::Update(time, scene);
	}

	void RiggedMeshRenderer::Render(RenderParam& param, int instances)
	{
		const auto& submeshes = m_riggedMesh->GetSubmeshes();

		if (param.UseFrustumCulling)
		{
			// Perform frustum culling
			BoundingBox boundsWorld;
			m_riggedMesh->GetBounds().Transform(boundsWorld, m_transformCache);
			if (param.ViewFrustumWorld->Contains(boundsWorld) == ContainmentType::DISJOINT)
			{
				return;
			}
		}

		ObjectConstants objectConstants;
		objectConstants.World = m_transformCache.Transpose();
		objectConstants.PrevWorld = m_prevTransformCache.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
		param.CommandList->IASetVertexBuffers(0, 1, &m_riggedMesh->VertexBufferView());
		param.CommandList->IASetIndexBuffer(&m_riggedMesh->IndexBufferView());
		param.CommandList->IASetPrimitiveTopology(m_topology);
		param.CommandList->OMSetStencilRef(static_cast<UINT>(m_drawOutline) << 7);

		auto& uploaders = m_constantBuffers[param.FrameResourceIndex];
		auto& prevUploaders = m_prevConstantBuffers[param.FrameResourceIndex];

		if (m_constantBuffersDirty)
		{
			// Update bone constants
			for (size_t index = 0; index < submeshes.size(); ++index)
			{
				std::vector<Matrix4x4> boneTransforms;
				PopulateTransforms(static_cast<int>(index), boneTransforms);

				BoneConstants boneConstants;
				memcpy(boneConstants.BoneTransforms, boneTransforms.data(), boneTransforms.size() * sizeof(Matrix4x4));
				uploaders[index]->CopyData(0, boneConstants);
				prevUploaders[index]->CopyData(0, m_boneConstantsCache[index]);
				memcpy(&m_boneConstantsCache[index], &boneConstants, sizeof(BoneConstants));
			}
			m_constantBuffersDirty = false;
		}

		for (size_t index = 0; index < submeshes.size(); ++index)
		{
			const auto& submesh = submeshes[index];

			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::BonesCBV, uploaders[index]->Resource()->GetGPUVirtualAddress());
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PrevBonesCBV, prevUploaders[index]->Resource()->GetGPUVirtualAddress());

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

			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instances, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
		}
	}

	RiggedMesh* RiggedMeshRenderer::GetMesh() const
	{
		return m_riggedMesh;
	}

	void RiggedMeshRenderer::SetMesh(RiggedMesh* mesh)
	{
		m_riggedMesh = mesh;

		size_t numSubmeshes = mesh->GetSubmeshes().size();
		for (auto& buffer : m_constantBuffers)
		{
			buffer.resize(numSubmeshes);
			for (auto& subBuffer : buffer)
			{
				subBuffer = std::make_unique<UploadBuffer<BoneConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
			}
		}
		for (auto& buffer : m_prevConstantBuffers)
		{
			buffer.resize(numSubmeshes);
			for (auto& subBuffer : buffer)
			{
				subBuffer = std::make_unique<UploadBuffer<BoneConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
			}
		}
		m_boneConstantsCache.resize(numSubmeshes);
	}

	void RiggedMeshRenderer::SetAnimation(AnimationClip* animationClip, bool loop, bool forcePlay)
	{
		if (!forcePlay && m_animation == animationClip)
		{
			return;
		}

		// If the animation is not blending
		if (m_transitionFactor > 1.0f)
		{
			m_prevAnimation = m_animation;
			m_prevAnimationTime = m_animationTime;
			m_animationTime = 0.0f;
			m_transitionFactor = 0.0f;
		}
		// If the animation is blending, but the new animation is previous one
		else if (animationClip == m_prevAnimation)
		{
			m_prevAnimation = m_animation;
			m_transitionFactor = 1.0f - m_transitionFactor;
			std::swap(m_animationTime, m_prevAnimationTime);
		}
		// If the animation is blending, but the new animation is different from previous one
		else
		{
			m_animationTime = 0.0f;
		}
		m_animation = animationClip;
		m_loop = loop;
	}

	ID3D12PipelineState* RiggedMeshRenderer::GetPipelineState() const
	{
		return m_shader->RiggedPipelineState();
	}

	ID3D12PipelineState* RiggedMeshRenderer::GetShadowPipelineState() const
	{
		return m_shader->RiggedShadowPipelineState();
	}

	static constexpr float SmoothStep(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}

	Matrix4x4 RiggedMeshRenderer::PopulateTransform(std::string_view boneName)
	{
		std::vector<std::string> names;
		std::vector<Matrix4x4> offsets;
		std::vector<Matrix4x4> out;

		names.emplace_back(boneName.data());
		offsets.emplace_back(Matrix4x4::Identity);

		if (m_animation == nullptr)
		{
			out.emplace_back(Matrix4x4::Identity);
		}
		else
		{
			float animationTime = m_loop ? fmodf(m_animationTime, m_animation->GetAnimationDuration()) : m_animationTime;
			m_animation->PopulateTransforms(animationTime, names, offsets, out);
		}
		if (m_transitionFactor < 1.0f && m_prevAnimation != nullptr)
		{
			std::vector<Matrix4x4> prevTransforms;
			m_prevAnimation->PopulateTransforms(m_prevAnimationTime, names, offsets, prevTransforms);
			float t = SmoothStep(std::clamp(m_transitionFactor, 0.0f, 1.0f));
			out[0] = Matrix4x4::Lerp(prevTransforms[0], out[0], t);
		}
		return out[0];
	}

	void RiggedMeshRenderer::PopulateTransforms(int submeshIndex, std::vector<Matrix4x4>& out)
	{
		if (m_animation == nullptr)
		{
			m_riggedMesh->PopulateTransforms(submeshIndex, out);
		}
		else
		{
			float animationTime = m_loop ? fmodf(m_animationTime, m_animation->GetAnimationDuration()) : m_animationTime;
			m_riggedMesh->PopulateTransforms(submeshIndex, *m_animation, animationTime, out);
		}
		if (m_transitionFactor < 1.0f && m_prevAnimation != nullptr)
		{
			std::vector<Matrix4x4> prevTransforms;
			m_riggedMesh->PopulateTransforms(submeshIndex, *m_prevAnimation, m_prevAnimationTime, prevTransforms);
			float t = SmoothStep(std::clamp(m_transitionFactor, 0.0f, 1.0f));
			for (size_t i = 0; i < out.size(); ++i)
			{
				out[i] = Matrix4x4::Lerp(prevTransforms[i], out[i], t);
			}
		}
	}
}