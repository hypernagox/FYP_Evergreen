#include "pch.h"
#include "texture.h"

namespace udsdx
{
	Texture::Texture(std::wstring_view path, ID3D12Device* device, ID3D12CommandQueue* commandQueue) : ResourceObject(path)
	{
		ResourceUploadBatch resourceUpload(device);
		D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
		WIC_LOADER_FLAGS wicFlags = WIC_LOADER_IGNORE_SRGB;

		resourceUpload.Begin();
		ThrowIfFailed(CreateWICTextureFromFileEx(
			device, resourceUpload, path.data(), 0, resourceFlags, wicFlags, m_texture.GetAddressOf()
		));

		auto uploadFinished = resourceUpload.End(commandQueue);
		uploadFinished.wait();
	}

	Texture::~Texture()
	{

	}

	void Texture::CreateShaderResourceView(ID3D12Device* device, DescriptorParam& descriptorParam)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = m_texture->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;

		m_srvCpu = descriptorParam.SrvCpuHandle;
		m_srvGpu = descriptorParam.SrvGpuHandle;

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvCpu);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSrvCpu() const
	{
		return m_srvCpu;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetSrvGpu() const
	{
		return m_srvGpu;
	}
}