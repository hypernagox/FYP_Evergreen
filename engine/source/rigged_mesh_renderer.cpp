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
	void RiggedMeshRenderer::PostUpdate(const Time& time, Scene& scene)
	{
		RendererBase::PostUpdate(time, scene);

		int submeshCount = m_riggedMesh ? static_cast<int>(std::min(m_riggedMesh->GetSubmeshes().size(), m_materials.size())) : 0;
		for (int i = 0; i < submeshCount; ++i)
		{
			scene.EnqueueRenderObject(this, m_renderGroup, m_materials[i].GetShader()->RiggedPipelineState(), m_materials[i].GetShader()->DeferredPipelineState(), i);
			if (m_castShadow == true)
			{
				scene.EnqueueRenderShadowObject(this, m_materials[i].GetShader()->RiggedShadowPipelineState(), i);
			}
		}
	}

	void RiggedMeshRenderer::Update(const Time& time, Scene& scene)
	{
		m_animationTime += time.deltaTime;
		m_prevAnimationTime += time.deltaTime;
		m_transitionFactor += time.deltaTime / 0.2f;
		m_constantBuffersDirty = true;

		RendererBase::Update(time, scene);
	}

	void RiggedMeshRenderer::OnDrawGizmos(const Camera* target)
	{	
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		float screenRatio = screenSize.x / screenSize.y;
		
		// Perform frustum culling
		BoundingBox boundsWorld;
		m_riggedMesh->GetBounds().Transform(boundsWorld, m_transformCache);
		if (nullptr == m_animation || target->GetViewFrustumWorld(screenRatio)->Contains(boundsWorld) == ContainmentType::DISJOINT)
		{
			return;
		}

		std::vector<Matrix4x4> boneTransforms;
		PopulateTransforms(boneTransforms);
		const auto& boneParents = m_animation->GetBoneParents();

		std::vector<ImVec2> boneScreenPositions(boneTransforms.size());

		for (size_t index = 0; index < boneParents.size(); ++index)
		{
			const auto& bone = boneTransforms[index];

			Vector3 worldPosition = Vector3::Transform(Vector3(bone.m[0][3], bone.m[1][3], bone.m[2][3]), m_transformCache);
			Vector2 screenPosition = target->ToScreenPosition(worldPosition);
			boneScreenPositions[index] = ImVec2(screenPosition.x, screenPosition.y);
		}

		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		for (size_t index = 0; index < boneParents.size(); ++index)
		{
			const auto& parentIndex = boneParents[index];

			ImDrawList* drawList = ImGui::GetBackgroundDrawList();
			drawList->AddRectFilled(
				ImVec2(boneScreenPositions[index].x - 2.0f, boneScreenPositions[index].y - 2.0f),
				ImVec2(boneScreenPositions[index].x + 2.0f, boneScreenPositions[index].y + 2.0f),
				IM_COL32(255, 255, 0, 255));

			if (boneParents[index] >= 0)
			{
				drawList->AddLine(
					boneScreenPositions[index],
					boneScreenPositions[parentIndex],
					IM_COL32(255, 255, 0, 255), 2.0f);
			}
		}

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

	void RiggedMeshRenderer::Render(RenderParam& param, int parameter)
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

		param.CommandList->SetGraphicsRootConstantBufferView(RootParam::BonesCBV, uploaders[parameter]->Resource()->GetGPUVirtualAddress());
		param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PrevBonesCBV, prevUploaders[parameter]->Resource()->GetGPUVirtualAddress());

		for (UINT textureSrcIndex = 0; textureSrcIndex < m_materials[parameter].GetTextureCount(); ++textureSrcIndex)
		{
			const Texture* texture = m_materials[parameter].GetSourceTexture(textureSrcIndex);
			if (texture != nullptr)
			{
				param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0 + textureSrcIndex, texture->GetSrvGpu());
			}
		}

		const auto& submesh = submeshes[parameter];
		param.CommandList->DrawIndexedInstanced(submesh.IndexCount, 1, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
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
		if (m_transitionFactor > 1.0f || forcePlay)
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

	static constexpr float SmoothStep(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}

	void RiggedMeshRenderer::PopulateTransforms(std::vector<Matrix4x4>& out)
	{
		if (m_animation == nullptr)
		{
			out.emplace_back(Matrix4x4::Identity);
		}
		else
		{
			float animationTime = m_loop ? fmodf(m_animationTime, m_animation->GetAnimationDuration()) : m_animationTime;
			m_animation->PopulateTransforms(animationTime, out);
		}
		if (m_transitionFactor < 1.0f && m_prevAnimation != nullptr)
		{
			std::vector<Matrix4x4> prevTransforms;
			m_prevAnimation->PopulateTransforms(m_prevAnimationTime, prevTransforms);
			float t = SmoothStep(std::clamp(m_transitionFactor, 0.0f, 1.0f));
			for (size_t i = 0; i < out.size(); ++i)
			{
				out[i] = Matrix4x4::Lerp(prevTransforms[i], out[i], t);
			}
		}
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