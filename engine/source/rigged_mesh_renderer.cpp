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

		CacheBoneTransforms();

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

		const auto& boneParents = m_riggedMesh->GetBoneParents();
		std::vector<ImVec2> boneScreenPositions(m_boneTransformCache.size());

		for (size_t index = 0; index < m_boneTransformCache.size(); ++index)
		{
			const auto& bone = m_boneTransformCache[index];

			Vector3 worldPosition = Vector3::Transform(Vector3(bone.m[3][0], bone.m[3][1], bone.m[3][2]), m_transformCache);
			Vector2 screenPosition = target->ToScreenPosition(worldPosition);
			boneScreenPositions[index] = ImVec2(screenPosition.x, screenPosition.y);
		}

		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		for (size_t index = 0; index < boneScreenPositions.size(); ++index)
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

		std::string submeshInfo;
		const auto& submeshes = m_riggedMesh->GetSubmeshes();
		if (!submeshes.empty())
		{
			submeshInfo = "Submesh Count: " + std::to_string(submeshes.size());
			for (size_t i = 0; i < submeshes.size(); ++i)
			{
				submeshInfo += "\nSubmesh " + submeshes[i].Name + ": " +
					"Index Count: " + std::to_string(submeshes[i].IndexCount) +
					", Start Index: " + std::to_string(submeshes[i].StartIndexLocation) +
					", Base Vertex: " + std::to_string(submeshes[i].BaseVertexLocation);
			}
		}
		drawList->AddText(
			ImVec2(cornersScreen[0].x, cornersScreen[0].y),
			drawColor,
			submeshInfo.c_str());
	}

	void RiggedMeshRenderer::Render(RenderParam& param, int parameter)
	{
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

		const auto& submeshes = m_riggedMesh->GetSubmeshes();
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
				for (size_t boneIndex = 0; boneIndex < m_submeshBoneMapCache[index].size(); ++boneIndex)
				{
					Matrix4x4 boneTransform = m_boneTransformCache[m_submeshBoneMapCache[index][boneIndex]];
					Matrix4x4 finalTransform = submeshes[index].BoneOffsets[boneIndex] * boneTransform;
					boneTransforms.emplace_back(finalTransform.Transpose());
				}

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

		const auto& submeshes = mesh->GetSubmeshes();
		size_t numSubmeshes = mesh->GetSubmeshes().size();

		if (m_animation != nullptr)
		{
			m_animation->GetAnimationClip()->PopulateBoneMap(m_riggedMesh->GetBoneNames(), m_boneMapCache);
		}
		if (m_prevAnimation != nullptr)
		{
			m_prevAnimation->GetAnimationClip()->PopulateBoneMap(m_riggedMesh->GetBoneNames(), m_prevBoneMapCache);
		}

		for (size_t index = 0; index < numSubmeshes; ++index)
		{
			auto& cache = m_submeshBoneMapCache.emplace_back();
			cache.resize(submeshes[index].BoneNodeIDs.size());
			for (size_t boneIndex = 0; boneIndex < submeshes[index].BoneNodeIDs.size(); ++boneIndex)
			{
				cache[boneIndex] = m_riggedMesh->GetBoneIndex(submeshes[index].BoneNodeIDs[boneIndex]);
			}
		}

		for (size_t index = 0; index < FrameResourceCount; ++index)
		{
			m_constantBuffers[index].resize(numSubmeshes);
			m_prevConstantBuffers[index].resize(numSubmeshes);
			for (size_t subIndex = 0; subIndex < numSubmeshes; ++subIndex)
			{
				m_constantBuffers[index][subIndex] = std::make_unique<UploadBuffer<BoneConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
				m_prevConstantBuffers[index][subIndex] = std::make_unique<UploadBuffer<BoneConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
			}
		}
		m_boneConstantsCache.resize(numSubmeshes);
	}

	void RiggedMeshRenderer::SetAnimation(const AnimationClip* animationClip, bool loop, bool forcePlay)
	{
		SetAnimation(&animationClip->GetAnimation(), loop, forcePlay);
	}

	void RiggedMeshRenderer::SetAnimation(const AnimationClip* animationClip, std::string_view animationName, bool loop, bool forcePlay)
	{
		SetAnimation(&animationClip->GetAnimation(animationName), loop, forcePlay);
	}

	void RiggedMeshRenderer::SetAnimation(const Animation* animation, bool loop, bool forcePlay)
	{
		if (!forcePlay && m_animation == animation)
		{
			return;
		}

		// If the animation is not blending
		if (m_transitionFactor >= 1.0f || forcePlay)
		{
			m_prevAnimation = m_animation;
			m_prevAnimationTime = m_animationTime;
			m_animationTime = 0.0f;
			m_transitionFactor = 0.0f;
			m_prevBoneMapCache = m_boneMapCache;
			animation->GetAnimationClip()->PopulateBoneMap(m_riggedMesh->GetBoneNames(), m_boneMapCache);
		}
		// If the animation is blending, but the new animation is previous one
		else if (animation == m_prevAnimation)
		{
			m_prevAnimation = m_animation;
			m_transitionFactor = 1.0f - m_transitionFactor;
			std::swap(m_animationTime, m_prevAnimationTime);
			std::swap(m_boneMapCache, m_prevBoneMapCache);
		}
		// If the animation is blending, but the new animation is different from previous one
		else
		{
			m_animationTime = 0.0f;
			animation->GetAnimationClip()->PopulateBoneMap(m_riggedMesh->GetBoneNames(), m_boneMapCache);
		}

		m_animation = animation;
		m_loop = loop;
	}

	void RiggedMeshRenderer::SetTransitionFactor(float factor)
	{
		m_transitionFactor = factor;
	}

	void RiggedMeshRenderer::SetBoneModifier(std::string_view boneName, const Matrix4x4& transform)
	{
		m_boneModifiers[boneName] = transform;
	}

	const Matrix4x4& RiggedMeshRenderer::GetBoneTransform(std::string_view boneName) const
	{
		int boneIndex = m_riggedMesh->GetBoneIndex(boneName);
		if (boneIndex < 0 || boneIndex >= static_cast<int>(m_boneTransformCache.size()))
		{
			static Matrix4x4 identity;
			return identity; // Return identity matrix if bone not found
		}
		return m_boneTransformCache[boneIndex];
	}

	void RiggedMeshRenderer::ClearBoneModifiers()
	{
		m_boneModifiers.clear();
	}

	void RiggedMeshRenderer::CacheBoneTransforms()
	{
		if (m_animation == nullptr)
		{
			m_riggedMesh->PopulateTransforms(m_boneTransformCache);
		}
		else
		{
			float animationTime = m_loop ? fmodf(m_animationTime, m_animation->GetAnimationDuration()) : m_animationTime;
			m_animation->PopulateTransforms(animationTime, m_boneMapCache, m_boneTransformCache, m_boneModifiers);
		}
		if (m_transitionFactor < 1.0f && m_prevAnimation != nullptr)
		{
			std::vector<Matrix4x4> prevTransforms;
			m_prevAnimation->PopulateTransforms(m_prevAnimationTime, m_prevBoneMapCache, prevTransforms, m_boneModifiers);
			float t = SmoothStep(std::clamp(m_transitionFactor, 0.0f, 1.0f));
			for (size_t i = 0; i < m_boneTransformCache.size(); ++i)
			{
				m_boneTransformCache[i] = Matrix4x4::Lerp(prevTransforms[i], m_boneTransformCache[i], t);
			}
		}
	}

	bool RiggedMeshRenderer::IsAnimationPlaying() const
	{
		return m_animation != nullptr && (m_loop || m_animationTime < m_animation->GetAnimationDuration());
	}
}