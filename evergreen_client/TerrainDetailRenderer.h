#pragma once

#include "pch.h"

class TerrainDetail;

class TerrainDetailRenderer : public udsdx::RendererBase
{
public:
	TerrainDetailRenderer(const std::shared_ptr<udsdx::SceneObject>& object);

	void SetTerrainDetail(TerrainDetail* terrainDetail) { m_terrainDetail = terrainDetail; }

	void Render(udsdx::RenderParam& param, int instances) override;

	ID3D12PipelineState* GetPipelineState() const override { return m_shader->DefaultPipelineState(); }
	ID3D12PipelineState* GetShadowPipelineState() const override { return m_shader->ShadowPipelineState(); }

private:
	TerrainDetail* m_terrainDetail = nullptr;
};