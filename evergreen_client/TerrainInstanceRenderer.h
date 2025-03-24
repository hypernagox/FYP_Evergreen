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
	void SetMesh(udsdx::Mesh* mesh);

	TerrainData* GetTerrainData() const;
	void SetTerrainData(TerrainData* terrainData, std::string_view prototypeName);

	virtual ID3D12PipelineState* GetPipelineState() const override;
	virtual ID3D12PipelineState* GetShadowPipelineState() const override;

protected:
	udsdx::Mesh* m_mesh = nullptr;
	TerrainData* m_terrainData = nullptr;
	std::string m_prototypeName;

	std::vector<std::array<std::unique_ptr<UploadBuffer<udsdx::Matrix4x4>>, udsdx::FrameResourceCount>> m_instanceUploadBuffer;
};