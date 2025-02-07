#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class RiggedMesh;

	class RiggedMeshRenderer : public RendererBase
	{
	public:
		struct BoneConstants
		{
			UINT FrameStride = 0;
			UINT SubmeshIndex = 0;
			UINT FrameIndex = 0;
			UINT PrevFrameIndex = 0;
			float FrameFrac = 0.0f;
			float PrevFrameFrac = 0.0f;
			UINT TransitionFrameIndex = 0;
			UINT PrevTransitionFrameIndex = 0;
			float TransitionFrameFrac = 0.0f;
			float PrevTransitionFrameFrac = 0.0f;
			float TransitionFactor = 0.0f;
			float PrevTransitionFactor = 0.0f;
		};

	public:
		RiggedMeshRenderer(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Update(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int instances = 1);

	public:
		RiggedMesh* GetMesh() const;
		void SetMesh(RiggedMesh* mesh);
		void SetAnimation(std::string_view animationName);

		virtual ID3D12PipelineState* GetPipelineState() const override;
		virtual ID3D12PipelineState* GetShadowPipelineState() const override;

		void PopulateTransforms(int submeshIndex, std::vector<Matrix4x4>& out);

	protected:
		RiggedMesh* m_riggedMesh = nullptr;

		std::string m_animationName{};
		std::string m_prevAnimationName{};
		float m_animationTime = 0.0f;
		float m_prevAnimationTime = 0.0f;
		float m_transitionFactor = 0.0f;

		std::array<std::vector<std::unique_ptr<UploadBuffer<BoneConstants>>>, FrameResourceCount> m_constantBuffers;
		bool m_constantBuffersDirty = true;
		BoneConstants m_boneConstantCache{};
	};
}