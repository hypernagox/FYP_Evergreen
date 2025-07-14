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
		bool IsAnimationPlaying() const;

		Matrix4x4 PopulateTransform(std::string_view boneName);
		void PopulateTransforms(std::vector<Matrix4x4>& out);
		void PopulateTransforms(int submeshIndex, std::vector<Matrix4x4>& out);

	protected:
		RiggedMesh* m_riggedMesh = nullptr;

		const Animation* m_animation = nullptr;
		const Animation* m_prevAnimation = nullptr;

		std::vector<std::vector<int>> m_boneMapCache;
		std::vector<std::vector<int>> m_prevBoneMapCache;

		bool m_loop = false;
		float m_animationTime = 0.0f;
		float m_prevAnimationTime = 0.0f;
		float m_transitionFactor = 0.0f;

		std::array<std::vector<std::unique_ptr<UploadBuffer<BoneConstants>>>, FrameResourceCount> m_constantBuffers;
		std::array<std::vector<std::unique_ptr<UploadBuffer<BoneConstants>>>, FrameResourceCount> m_prevConstantBuffers;
		std::vector<BoneConstants> m_boneConstantsCache;
		bool m_constantBuffersDirty = true;
	};
}