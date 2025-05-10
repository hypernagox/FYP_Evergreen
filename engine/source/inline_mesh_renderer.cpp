#include "pch.h"
#include "frame_resource.h"
#include "inline_mesh_renderer.h"
#include "shader.h"
#include "material.h"
#include "texture.h"

namespace udsdx
{
	InlineMeshRenderer::InlineMeshRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
	{
	}

	void InlineMeshRenderer::Render(RenderParam& param, int instances)
	{
		ObjectConstants objectConstants;
		objectConstants.World = m_transformCache.Transpose();
		objectConstants.PrevWorld = m_prevTransformCache.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
		param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0, m_materials[0]->GetSourceTexture()->GetSrvGpu());

		param.CommandList->IASetVertexBuffers(0, 0, nullptr);
		param.CommandList->IASetIndexBuffer(nullptr);
		param.CommandList->IASetPrimitiveTopology(m_topology);
		param.CommandList->OMSetStencilRef(static_cast<UINT>(m_drawOutline) << 7);

		param.CommandList->DrawInstanced(m_vertexCount, instances, 0, 0);
	}

	void InlineMeshRenderer::SetVertexCount(unsigned int value)
	{
		m_vertexCount = value;
	}

	unsigned int InlineMeshRenderer::GetVertexCount() const
	{
		return m_vertexCount;
	}

	ID3D12PipelineState* InlineMeshRenderer::GetPipelineState() const
	{
		return m_shader->DefaultPipelineState();
	}

	ID3D12PipelineState* InlineMeshRenderer::GetShadowPipelineState() const
	{
		return m_shader->ShadowPipelineState();
	}
}