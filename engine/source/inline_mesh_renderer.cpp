#include "pch.h"
#include "frame_resource.h"
#include "inline_mesh_renderer.h"
#include "shader.h"
#include "material.h"
#include "texture.h"
#include "scene.h"

namespace udsdx
{
	void InlineMeshRenderer::PostUpdate(const Time& time, Scene& scene)
	{
		RendererBase::PostUpdate(time, scene);

		scene.EnqueueRenderObject(this, m_renderGroup, m_materials[0].GetShader()->DefaultPipelineState(), m_materials[0].GetShader()->DeferredPipelineState(), 0);
		if (m_castShadow == true)
		{
			scene.EnqueueRenderShadowObject(this, m_materials[0].GetShader()->ShadowPipelineState(), 0);
		}
	}

	void InlineMeshRenderer::Render(RenderParam& param, int parameter)
	{
		ObjectConstants objectConstants;
		objectConstants.World = m_transformCache.Transpose();
		objectConstants.PrevWorld = m_prevTransformCache.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
		param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0, m_materials[0].GetSourceTexture()->GetSrvGpu());

		param.CommandList->IASetVertexBuffers(0, 0, nullptr);
		param.CommandList->IASetIndexBuffer(nullptr);
		param.CommandList->IASetPrimitiveTopology(m_topology);

		param.CommandList->DrawInstanced(m_vertexCount, 1, 0, 0);
	}

	void InlineMeshRenderer::SetVertexCount(unsigned int value)
	{
		m_vertexCount = value;
	}

	unsigned int InlineMeshRenderer::GetVertexCount() const
	{
		return m_vertexCount;
	}
}