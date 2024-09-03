#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	class Texture : public ResourceObject
	{
	public:
		Texture(std::wstring_view path, ID3D12Device* device, ID3D12CommandQueue* commandQueue);
		~Texture();

	public:
		void CreateShaderResourceView(ID3D12Device* device, DescriptorParam& descriptorParam);

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpu() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpu() const;

	private:
		ComPtr<ID3D12Resource> m_texture;

		D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpu;
	};
}