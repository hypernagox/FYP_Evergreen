#pragma once

#include "pch.h"

class GizmoBoxRenderer : public udsdx::RendererBase
{
public:
	GizmoBoxRenderer(const std::shared_ptr<udsdx::SceneObject>& object);

public:
	void Render(udsdx::RenderParam& param, int instances = 1) override;

	void BuildPipelineState();
	ID3D12PipelineState* GetPipelineState() const override { return m_pipelineState.Get(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return nullptr; }

	Vector3 GetSize() const { return m_size; }
	void SetSize(const Vector3& size) { m_size = size; }
	Vector3 GetOffset() const { return m_offset; }
	void SetOffset(const Vector3& offset) { m_offset = offset; }

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	Vector3 m_size = Vector3::One;
	Vector3 m_offset = Vector3::Zero;
};

