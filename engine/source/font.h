#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	class Font : public ResourceObject
	{
	public:
		Font(std::wstring_view path);
		void CreateShaderResourceView(ID3D12Device* device, DescriptorParam& descriptorParam);
		DirectX::SpriteFont* GetSpriteFont() const { return m_spriteFont.get(); }

	private:
		ComPtr<ID3DBlob> m_fontData;

		std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	};
}