#include "pch.h"
#include "font.h"
#include "core.h"

namespace udsdx
{
	Font::Font(std::wstring_view path)
	{
		// load the font data from file
		ThrowIfFailed(::D3DReadFileToBlob(path.data(), &m_fontData));
	}

	void Font::CreateShaderResourceView(ID3D12Device* device, DescriptorParam& descriptorParam)
	{
		ResourceUploadBatch uploadBatch(device);

		uploadBatch.Begin();

		m_spriteFont = std::make_unique<DirectX::SpriteFont>(
			device,
			uploadBatch,
			reinterpret_cast<uint8_t const*>(m_fontData->GetBufferPointer()),
			static_cast<size_t>(m_fontData->GetBufferSize()),
			descriptorParam.SrvCpuHandle,
			descriptorParam.SrvGpuHandle
		);
		m_spriteFont->SetDefaultCharacter(L'?');

		auto eventFinishied = uploadBatch.End(INSTANCE(Core)->GetCommandQueue());
		eventFinishied.wait();

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
	}
}
