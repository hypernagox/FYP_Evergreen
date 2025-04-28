#include "pch.h"
#include "texture.h"
#include "debug_console.h"

#include <DirectXTex.h>

namespace udsdx
{
	Texture::Texture(std::wstring_view path, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : ResourceObject(path)
	{
		// Set the name of the texture (with file name except directory)
		std::filesystem::path pathTexture(path);
		m_name = pathTexture.filename().string();

		std::filesystem::path pathDds(path);
		pathDds.replace_extension(L".ddscache");

		bool loadFromCache = false;
		if (std::filesystem::exists(pathDds))
		{
			// Check when the file was last modified
			auto lastWriteTime = std::filesystem::last_write_time(path);
			auto lastWriteTimeCache = std::filesystem::last_write_time(pathDds);
			if (lastWriteTimeCache >= lastWriteTime)
			{
				loadFromCache = true;
			}
		}

		ScratchImage image;
		if (loadFromCache)
		{
			ThrowIfFailed(::LoadFromDDSFile(pathDds.c_str(), DDS_FLAGS_NONE, nullptr, image));
		}
		else
		{
			if (path.ends_with(L".tga"))
			{
				ThrowIfFailed(::LoadFromTGAFile(path.data(), nullptr, image));
			}
			else
			{
				ThrowIfFailed(::LoadFromWICFile(path.data(), WIC_FLAGS_NONE, nullptr, image));
			}

			ScratchImage compressedImage;
			CompressOptions compressOptions = {};
			compressOptions.flags = TEX_COMPRESS_DEFAULT;
			compressOptions.threshold = TEX_THRESHOLD_DEFAULT;
			compressOptions.alphaWeight = TEX_ALPHA_WEIGHT_DEFAULT;

			ScratchImage mipChain;
			const int mipChainLevels = std::log2<int>(std::max<int>(image.GetMetadata().width, image.GetMetadata().height)) + 1;
			// Generate MipMaps
			ThrowIfFailed(::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_DEFAULT, mipChainLevels, mipChain));
			// BC3 Compression
			ThrowIfFailed(::CompressEx(mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), DXGI_FORMAT_BC3_UNORM, compressOptions, compressedImage, [&](size_t, size_t) { return true; }));

			ThrowIfFailed(::SaveToDDSFile(compressedImage.GetImages(), compressedImage.GetImageCount(), compressedImage.GetMetadata(), DDS_FLAGS_NONE, pathDds.c_str()));
			std::filesystem::last_write_time(pathDds, std::filesystem::last_write_time(path));

			image = std::move(compressedImage);
			DebugConsole::Log("\tTexture compressed and cached: " + pathDds.string());
		}

		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		ThrowIfFailed(::CreateTextureEx(device, image.GetMetadata(), D3D12_RESOURCE_FLAG_NONE, CREATETEX_IGNORE_SRGB, &m_texture));
		ThrowIfFailed(::PrepareUpload(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), subresources));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, static_cast<unsigned int>(subresources.size()));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize + 3 & ~3),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_textureUpload.GetAddressOf())));

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources(commandList,
			m_texture.Get(), m_textureUpload.Get(),
			0, 0, static_cast<unsigned int>(subresources.size()),
			subresources.data());
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_size = Vector2Int(static_cast<int32_t>(image.GetMetadata().width), static_cast<int32_t>(image.GetMetadata().height));
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

	void Texture::DisposeUploaders()
	{
		m_textureUpload.Reset();
	}
}