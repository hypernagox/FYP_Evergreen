#pragma once

#include "pch.h"

class TerrainDetail;

class TerrainDetailRenderer : public udsdx::RendererBase
{
public:
	void SetTerrainDetail(TerrainDetail* terrainDetail) { m_terrainDetail = terrainDetail; }

	void OnInitialize() override;
	void PostUpdate(const udsdx::Time& time, udsdx::Scene& scene) override;
	void Render(udsdx::RenderParam& param, int instances) override;

private:
	TerrainDetail* m_terrainDetail = nullptr;
};