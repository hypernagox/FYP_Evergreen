#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class InlineMeshRenderer : public RendererBase
	{
	public:
		virtual void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int parameter) override;

	public:
		void SetVertexCount(unsigned int value);
		unsigned int GetVertexCount() const;

	public:
		unsigned int m_vertexCount = 0;
	};
}