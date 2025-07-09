#pragma once

#include "pch.h"

class GizmoSectorRenderer : public udsdx::RendererBase
{
public:
	void OnInitialize() override;
	void Render(udsdx::RenderParam& param, int instances = 1) override;

	void BuildPipelineState();
	ID3D12PipelineState* GetPipelineState() const override { return m_pipelineState.Get(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return nullptr; }

	void SetRadius(float radius) { m_radius = radius; }
	void SetAngle(float angle) { m_angle = angle; }

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;

	float m_radius = 1.0f;
	float m_angle = 90.0f;
};