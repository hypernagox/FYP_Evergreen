#pragma once

#include "pch.h"

using namespace udsdx;

class MonsterHPPanel : public udsdx::RendererBase
{
public:
	MonsterHPPanel(const std::shared_ptr<SceneObject>& object);

public:
	void Render(udsdx::RenderParam& param, int instances = 1) override;

	void BuildPipelineState();
	ID3D12PipelineState* GetPipelineState() const override { return m_pipelineState.Get(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return nullptr; }

	float GetHPFraction() const noexcept { return m_hpFraction; }
	void SetHPFraction(float value) noexcept { m_hpFraction = value; }

protected:
	static ComPtr<ID3D12PipelineState> m_pipelineState;

	float m_hpFraction = 1.0f;
};

