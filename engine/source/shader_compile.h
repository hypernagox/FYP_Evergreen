#pragma once

#include "pch.h"

namespace udsdx
{
	class ShaderIncludeHandler : public IDxcIncludeHandler
	{
	public:
		ShaderIncludeHandler(const std::wstring& shaderDirectory);

		HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
		ULONG STDMETHODCALLTYPE AddRef() override;
		ULONG STDMETHODCALLTYPE Release() override;

	private:
		std::wstring m_shaderDirectory;
		ULONG m_refCount = 1;
	};

	ComPtr<IDxcBlob> CompileShader(const std::wstring& filename, const std::span<std::wstring>& defines, const std::wstring& entrypoint, const std::wstring& target);
	ComPtr<IDxcBlob> CompileShaderFromMemory(const std::string& data, const std::span<std::wstring>& defines, const std::wstring& entrypoint, const std::wstring& target);
}