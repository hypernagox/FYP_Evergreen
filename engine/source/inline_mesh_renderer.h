#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class InlineMeshRenderer : public RendererBase
	{
	public:
		InlineMeshRenderer(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Render(RenderParam& param, int instances = 1) override;

	public:
		void SetVertexCount(unsigned int value);
		unsigned int GetVertexCount() const;

		virtual ID3D12PipelineState* GetPipelineState() const override;
		virtual ID3D12PipelineState* GetShadowPipelineState() const override;

	public:
		unsigned int m_vertexCount = 0;
	};
}