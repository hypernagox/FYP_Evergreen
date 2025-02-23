#pragma once

#include "pch.h"

class TerrainData
{
public:
	TerrainData(std::wstring_view instancesPath);

	void CreateBuffer(std::wstring_view instancesPath);
	void UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	D3D12_VERTEX_BUFFER_VIEW GetTransformBufferView() const;
	UINT GetPrototypeCount() const;
	UINT GetPrototypeInstanceBase(int index) const;
	UINT GetPrototypeInstanceCount(int index) const;

private:
	UINT m_prototypeCount = 0;
	UINT m_vertexByteStride = sizeof(udsdx::Matrix4x4);
	UINT m_vertexBufferByteSize = 0;

	ComPtr<ID3DBlob> m_bufferCPU;
	ComPtr<ID3D12Resource> m_bufferGpu;
	ComPtr<ID3D12Resource> m_bufferUpload;

	std::vector<std::pair<UINT, UINT>> m_baseCountPairs;
};