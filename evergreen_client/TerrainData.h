#pragma once

#include "pch.h"

class TerrainData
{
public:
	TerrainData(std::wstring_view instancesPath);

	void CreateBuffer(std::wstring_view instancesPath);
	void UploadBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void BuildDescriptors();

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetTransformGpuSrv(int index) const;
	UINT GetPrototypeCount() const;
	UINT GetPrototypeInstanceBase(int index) const;
	UINT GetPrototypeInstanceCount(int index) const;

private:
	UINT m_prototypeCount = 0;

	std::vector<ComPtr<ID3DBlob>> m_bufferCPU;
	std::vector<ComPtr<ID3D12Resource>> m_bufferGpu;
	std::vector<ComPtr<ID3D12Resource>> m_bufferUpload;

	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> m_desciptorCpuSrv;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> m_desciptorGpuSrv;

	std::vector<std::pair<UINT, UINT>> m_baseCountPairs;
};