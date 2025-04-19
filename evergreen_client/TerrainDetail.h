#pragma once

#include "pch.h"

class HeightMap;

class TerrainDetail
{
public:
	TerrainDetail(HeightMap* heightMap, std::wstring_view filename, LONG sourceWidth, LONG sourceHeight, UINT segmentation, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

public:
	udsdx::Mesh* GetMesh() const { return m_detailMesh.get(); }
	UINT GetSegmentation() const { return m_segmentation; }
	UINT GetIndexBase(UINT segmentX, UINT segmentY) const { return m_submeshes[segmentX][segmentY].first; }
	UINT GetIndexCount(UINT segmentX, UINT segmentY) const { return m_submeshes[segmentX][segmentY].second; }

private:
	UINT m_segmentation = 0;
	std::unique_ptr<udsdx::Mesh> m_detailMesh;
	std::vector<std::vector<std::pair<UINT, UINT>>> m_submeshes;
};

