#pragma once

#include "pch.h"

class GizmoSphereRenderer : public udsdx::RendererBase
{
public:
	GizmoSphereRenderer(const std::shared_ptr<udsdx::SceneObject>& object);

public:
	void Render(udsdx::RenderParam& param, int instances = 1) override;

	void BuildPipelineState();
	ID3D12PipelineState* GetPipelineState() const override { return m_pipelineState.Get(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return nullptr; }

	float GetRadius() const { return m_radius; }
	void SetRadius(float radius) { m_radius = radius; }
	Vector3 GetOffset() const { return m_offset; }
	void SetOffset(const Vector3& offset) { m_offset = offset; }

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	float m_radius = 1.0f;
	Vector3 m_offset = Vector3::Zero;
};

