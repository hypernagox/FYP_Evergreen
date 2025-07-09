#pragma once

#include "pch.h"

class TerrainDetail;

class TerrainDetailRenderer : public udsdx::RendererBase
{
public:
	void SetTerrainDetail(TerrainDetail* terrainDetail) { m_terrainDetail = terrainDetail; }

	void OnInitialize() override;
	void Render(udsdx::RenderParam& param, int instances) override;

	ID3D12PipelineState* GetPipelineState() const override { return m_shader->DefaultPipelineState(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return m_shader->ShadowPipelineState(); }

private:
	TerrainDetail* m_terrainDetail = nullptr;
};