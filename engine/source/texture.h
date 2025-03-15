#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	class Texture : public ResourceObject
	{
	public:
		Texture(std::wstring_view path, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		~Texture();

	public:
		void CreateShaderResourceView(ID3D12Device* device, DescriptorParam& descriptorParam);

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpu() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpu() const;

		Vector2Int GetSize() const { return m_size; }
		int GetWidth() const { return m_size.x; }
		int GetHeight() const { return m_size.y; }

	private:
		ComPtr<ID3D12Resource> m_texture;
		ComPtr<ID3D12Resource> m_textureUpload;

		D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpu;

		Vector2Int m_size = Vector2Int(0, 0);
	};
}