#include "pch.h"
#include "shader_compile.h"
#include "debug_console.h"

namespace udsdx
{
    static ComPtr<IDxcUtils> g_pUtils;
    static ComPtr<IDxcCompiler3> g_pCompiler;

    ShaderIncludeHandler::ShaderIncludeHandler(const std::wstring& shaderDirectory) : m_shaderDirectory(shaderDirectory)
    {
    }

    HRESULT STDMETHODCALLTYPE ShaderIncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
    {
        std::wstring filename = m_shaderDirectory + pFilename;
        ComPtr<IDxcBlobEncoding> pBlob;
        ThrowIfFailed(g_pUtils->LoadFile(filename.c_str(), nullptr, &pBlob));
        *ppIncludeSource = pBlob.Detach();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ShaderIncludeHandler::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == __uuidof(IDxcIncludeHandler)) {
            *ppvObject = static_cast<IDxcIncludeHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE ShaderIncludeHandler::AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE ShaderIncludeHandler::Release()
    {
        ULONG refCount = InterlockedDecrement(&m_refCount);
        if (refCount == 0) {
            delete this;
        }
        return refCount;
    }

    ComPtr<IDxcBlob> CompileShader(const std::wstring& filename, const std::span<std::wstring>& defines, const std::wstring& entrypoint, const std::wstring& target)
    {
        if (!g_pUtils) {
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_pUtils)));
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&g_pCompiler)));
        }

        ComPtr<IDxcIncludeHandler> pIncludeHandler{
            new ShaderIncludeHandler(filename.substr(0, filename.find_last_of(L"\\/") + 1))
        };

        ComPtr<IDxcBlobEncoding> pBlob;
        ThrowIfFailed(g_pUtils->LoadFile(filename.c_str(), nullptr, &pBlob));

        DxcBuffer source;
        source.Ptr = pBlob->GetBufferPointer();
        source.Size = pBlob->GetBufferSize();
        source.Encoding = DXC_CP_ACP;

        std::vector<LPCWSTR> args{
            L"-E", entrypoint.c_str(),
            L"-T", target.c_str(),
#ifdef PROFILE_ENABLE
            L"-Zi",
            L"-Qembed_debug"
#endif
        };

        for (const auto& define : defines) {
            args.push_back(L"-D");
            args.push_back(define.c_str());
        }

        ComPtr<IDxcResult> pResult;
        ThrowIfFailed(g_pCompiler->Compile(&source, args.data(), static_cast<UINT32>(args.size()), pIncludeHandler.Get(), IID_PPV_ARGS(&pResult)));

        ComPtr<IDxcBlobUtf8> pErrors;
        ThrowIfFailed(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));
        if (pErrors && pErrors->GetStringLength() > 0) {
            DebugConsole::Log(std::string((char*)pErrors->GetBufferPointer()));
        }

        HRESULT hrStatus;
        ThrowIfFailed(pResult->GetStatus(&hrStatus));
        ThrowIfFailed(hrStatus);

        ComPtr<IDxcBlob> pObject;
        ThrowIfFailed(pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pObject), nullptr));
        return pObject;
    }

    ComPtr<IDxcBlob> CompileShaderFromMemory(const std::string& data, const std::span<std::wstring>& defines, const std::wstring& entrypoint, const std::wstring& target)
    {
        if (!g_pUtils) {
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_pUtils)));
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&g_pCompiler)));
		}

        DxcBuffer source;
        source.Ptr = data.c_str();
        source.Size = data.size();
        source.Encoding = DXC_CP_ACP;

        std::vector<LPCWSTR> args{
            L"-E", entrypoint.c_str(),
            L"-T", target.c_str(),
#ifdef PROFILE_ENABLE
            L"-Zi",
            L"-Qembed_debug"
#endif
        };

        for (const auto& define : defines) {
			args.push_back(L"-D");
			args.push_back(define.c_str());
		}

        ComPtr<IDxcResult> pResult;
        ThrowIfFailed(g_pCompiler->Compile(&source, args.data(), static_cast<UINT32>(args.size()), nullptr, IID_PPV_ARGS(&pResult)));

        ComPtr<IDxcBlobUtf8> pErrors;
        ThrowIfFailed(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));
        if (pErrors && pErrors->GetStringLength() > 0) {
            DebugConsole::Log(std::string((char*)pErrors->GetBufferPointer()));
        }

        HRESULT hrStatus;
        ThrowIfFailed(pResult->GetStatus(&hrStatus));
        ThrowIfFailed(hrStatus);

        ComPtr<IDxcBlob> pObject;
        ThrowIfFailed(pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pObject), nullptr));
        return pObject;
    }
}