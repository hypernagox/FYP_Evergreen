#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class RiggedMesh;
	class AnimationClip;
	class Animation;

	class RiggedMeshRenderer : public RendererBase
	{
	public:
		struct BoneConstants
		{
			Matrix4x4 BoneTransforms[256];
		};

	public:
		virtual void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Update(const Time& time, Scene& scene) override;
		virtual void OnDrawGizmos(const Camera* target) override;
		virtual void Render(RenderParam& param, int parameter);

	public:
		RiggedMesh* GetMesh() const;
		void SetMesh(RiggedMesh* mesh);
		void SetAnimation(const AnimationClip* animationClip, bool loop = false, bool forcePlay = false);
		void SetAnimation(const AnimationClip* animationClip, std::string_view animationName, bool loop = false, bool forcePlay = false);
		void SetAnimation(const Animation* animation, bool loop = false, bool forcePlay = false);
		void SetTransitionFactor(float factor);
		void SetBoneModifier(std::string_view boneName, const Matrix4x4& transform);
		const Matrix4x4& GetBoneTransform(std::string_view boneName) const;
		void ClearBoneModifiers();
		void CacheBoneTransforms();
		bool IsAnimationPlaying() const;

	protected:
		RiggedMesh* m_riggedMesh = nullptr;

		const Animation* m_animation = nullptr;
		const Animation* m_prevAnimation = nullptr;

		// Stores bone indices for AnimationClip.
		// indexed by bone index of RiggedMesh.
		// (Rigged Mesh Bone Index -> Animation Clip Bone Index)
		std::vector<int> m_boneMapCache;
		std::vector<int> m_prevBoneMapCache;

		// Stores bone indices for each submesh.
		// indexed by bone index of bones of RiggedMesh.
		// (Submesh Bone Index -> Rigged Mesh Bone Index)
		std::vector<std::vector<int>> m_submeshBoneMapCache;

		// Stores temporal bone transforms (without offsets, not transposed). Can be updated by calling CacheBoneTransforms().
		// indexed by bone index of bones of RiggedMesh.
		// (Rigged Mesh Bone Index -> Bone Transform)
		std::vector<Matrix4x4> m_boneTransformCache;

		float m_animationTime = 0.0f;
		float m_prevAnimationTime = 0.0f;

		std::map<std::string_view, Matrix4x4> m_boneModifiers;

		bool m_loop = false;
		float m_transitionFactor = 0.0f;

		std::array<std::vector<std::unique_ptr<UploadBuffer<BoneConstants>>>, FrameResourceCount> m_constantBuffers;
		std::array<std::vector<std::unique_ptr<UploadBuffer<BoneConstants>>>, FrameResourceCount> m_prevConstantBuffers;
		std::vector<BoneConstants> m_boneConstantsCache;

		bool m_constantBuffersDirty = true;
	};
}