#pragma once

#include "pch.h"
#include <renderer_base.h>

class TerrainData;

class TerrainInstanceRenderer : public udsdx::RendererBase
{
public:
	TerrainInstanceRenderer(const std::shared_ptr<udsdx::SceneObject>& object);

public:
	virtual void Render(udsdx::RenderParam& param, int instances = 1) override;

public:
	void AddMesh(udsdx::Mesh* mesh);

	TerrainData* GetTerrainData() const;
	void SetTerrainData(TerrainData* terrainData);

	virtual ID3D12PipelineState* GetPipelineState() const override;
	virtual ID3D12PipelineState* GetShadowPipelineState() const override;

protected:
	std::vector<udsdx::Mesh*> m_meshes;
	TerrainData* m_terrainData = nullptr;
};