#include "pch.h"
#include "shader.h"
#include "debug_console.h"
#include "deferred_renderer.h"
#include "shader_compile.h"
#include "core.h"

namespace udsdx
{
	Shader::Shader(std::wstring_view path) : ResourceObject()
	{
		m_path = path;
	}

	void Shader::BuildPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		D3D12_DEPTH_STENCILOP_DESC stencilOpDesc;
		stencilOpDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		stencilOpDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		stencilOpDesc.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		stencilOpDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		psoDesc.pRootSignature = pRootSignature;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.StencilEnable = TRUE;
		psoDesc.DepthStencilState.FrontFace = stencilOpDesc;
		psoDesc.DepthStencilState.BackFace = stencilOpDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = DeferredRenderer::NUM_GBUFFERS;
		for (UINT i = 0; i < DeferredRenderer::NUM_GBUFFERS; ++i)
		{
			psoDesc.RTVFormats[i] = DeferredRenderer::GBUFFER_FORMATS[i];
		}
		psoDesc.SampleDesc.Count = 1; // m_4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = 0; // m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = DeferredRenderer::DEPTH_FORMAT;

		auto m_psByteCode = udsdx::CompileShader(m_path, {}, L"PS", L"ps_6_0");
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
			m_psByteCode->GetBufferSize()
		};

		// if HS and DS shaders exist, compile and set them
		ComPtr<IDxcBlob> hsByteCode = nullptr;
		ComPtr<IDxcBlob> dsByteCode = nullptr;
		try
		{
			hsByteCode = udsdx::CompileShader(m_path, {}, L"HS", L"hs_6_0");
			dsByteCode = udsdx::CompileShader(m_path, {}, L"DS", L"ds_6_0");

			psoDesc.HS =
			{
				reinterpret_cast<BYTE*>(hsByteCode->GetBufferPointer()),
				hsByteCode->GetBufferSize()
			};
			psoDesc.DS =
			{
				reinterpret_cast<BYTE*>(dsByteCode->GetBufferPointer()),
				dsByteCode->GetBufferSize()
			};
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
		catch (const DxException&) {}

		{
			auto m_vsByteCode = udsdx::CompileShader(m_path, {}, L"VS", L"vs_6_0");

			psoDesc.InputLayout = { Vertex::DescriptionTable, Vertex::DescriptionTableSize };
			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
				m_vsByteCode->GetBufferSize()
			};

			ComPtr<IDxcBlob> gsByteCode = nullptr;
			try
			{
				gsByteCode = udsdx::CompileShader(m_path, {}, L"GS", L"gs_6_0");
				psoDesc.GS =
				{
					reinterpret_cast<BYTE*>(gsByteCode->GetBufferPointer()),
					gsByteCode->GetBufferSize()
				};
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			}
			catch (const DxException&) {}

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_defaultPipelineState.GetAddressOf())
			));

			m_defaultPipelineState->SetName((m_path + L" (Default)").c_str());
			DebugConsole::Log("\tDefault shader compiled");
		}

		if (psoDesc.GS.BytecodeLength == 0)
		{
			std::wstring defines[] = {
				L"RIGGED"
			};

			auto m_vsByteCode = udsdx::CompileShader(m_path, defines, L"VS", L"vs_6_0");

			psoDesc.InputLayout = { RiggedVertex::DescriptionTable, RiggedVertex::DescriptionTableSize };
			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
				m_vsByteCode->GetBufferSize()
			};

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_riggedPipelineState.GetAddressOf())
			));

			m_riggedPipelineState->SetName((m_path + L" (Rigged)").c_str());
			DebugConsole::Log("\tRigged shader compiled");
		}

		psoDesc.NumRenderTargets = 0;
		for (UINT i = 0; i < DeferredRenderer::NUM_GBUFFERS; ++i)
		{
			psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}

		psoDesc.RasterizerState.DepthBias = 1024;
		psoDesc.RasterizerState.SlopeScaledDepthBias = 1.5f;

		{	
			std::wstring defines[] = {
				L"GENERATE_SHADOWS"
			};

			auto m_vsByteCode = udsdx::CompileShader(m_path, defines, L"VS", L"vs_6_0");
			auto m_psByteCode = udsdx::CompileShader(m_path, defines, L"ShadowPS", L"ps_6_0");

			psoDesc.InputLayout = { Vertex::DescriptionTable, Vertex::DescriptionTableSize };

			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
				m_vsByteCode->GetBufferSize()
			};
			psoDesc.PS =
			{
				reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
				m_psByteCode->GetBufferSize()
			};
			psoDesc.GS = {};

			ComPtr<IDxcBlob> gsByteCode = nullptr;
			try
			{
				gsByteCode = udsdx::CompileShader(m_path, defines, L"GS", L"gs_6_0");
				psoDesc.GS =
				{
					reinterpret_cast<BYTE*>(gsByteCode->GetBufferPointer()),
					gsByteCode->GetBufferSize()
				};
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			}
			catch (const DxException&) {}

			// if HS and DS shaders exist, compile and set them
			ComPtr<IDxcBlob> hsByteCode = nullptr;
			ComPtr<IDxcBlob> dsByteCode = nullptr;
			try
			{
				hsByteCode = udsdx::CompileShader(m_path, defines, L"HS", L"hs_6_0");
				dsByteCode = udsdx::CompileShader(m_path, defines, L"DS", L"ds_6_0");

				psoDesc.HS =
				{
					reinterpret_cast<BYTE*>(hsByteCode->GetBufferPointer()),
					hsByteCode->GetBufferSize()
				};
				psoDesc.DS =
				{
					reinterpret_cast<BYTE*>(dsByteCode->GetBufferPointer()),
					dsByteCode->GetBufferSize()
				};
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			}
			catch (const DxException&) {}

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_shadowPipelineState.GetAddressOf())
			));

			m_shadowPipelineState->SetName((m_path + L" (Default Shadow)").c_str());
			DebugConsole::Log("\tShadow shader compiled");
		}

		if (psoDesc.GS.BytecodeLength == 0)
		{
			std::wstring defines[] = {
				L"RIGGED", L"GENERATE_SHADOWS"
			};

			auto m_vsByteCode = udsdx::CompileShader(m_path, defines, L"VS", L"vs_6_0");
			auto m_psByteCode = udsdx::CompileShader(m_path, defines, L"ShadowPS", L"ps_6_0");

			psoDesc.InputLayout = { RiggedVertex::DescriptionTable, RiggedVertex::DescriptionTableSize };

			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
				m_vsByteCode->GetBufferSize()
			};
			psoDesc.PS =
			{
				reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
				m_psByteCode->GetBufferSize()
			};
			psoDesc.GS = {};

			// if HS and DS shaders exist, compile and set them
			ComPtr<IDxcBlob> hsByteCode = nullptr;
			ComPtr<IDxcBlob> dsByteCode = nullptr;
			try
			{
				hsByteCode = udsdx::CompileShader(m_path, defines, L"HS", L"hs_6_0");
				dsByteCode = udsdx::CompileShader(m_path, defines, L"DS", L"ds_6_0");

				psoDesc.HS =
				{
					reinterpret_cast<BYTE*>(hsByteCode->GetBufferPointer()),
					hsByteCode->GetBufferSize()
				};
				psoDesc.DS =
				{
					reinterpret_cast<BYTE*>(dsByteCode->GetBufferPointer()),
					dsByteCode->GetBufferSize()
				};
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			}
			catch (const DxException&) {}

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_riggedShadowPipelineState.GetAddressOf())
			));

			m_riggedShadowPipelineState->SetName((m_path + L" (Rigged Shadow)").c_str());
			DebugConsole::Log("\tRigged shadow shader compiled");
		}
	}

	void Shader::BuildDeferredPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		D3D12_DEPTH_STENCILOP_DESC stencilOpDesc;
		stencilOpDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		stencilOpDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		stencilOpDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		stencilOpDesc.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

		psoDesc.InputLayout.pInputElementDescs = nullptr;
		psoDesc.InputLayout.NumElements = 0;
		psoDesc.pRootSignature = INSTANCE(Core)->GetRenderer()->GetRootSignature();
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.StencilEnable = true;
		psoDesc.DepthStencilState.StencilReadMask = 0x7f;
		psoDesc.DepthStencilState.StencilWriteMask = 0x00;
		psoDesc.DepthStencilState.FrontFace = stencilOpDesc;
		psoDesc.DepthStencilState.BackFace = stencilOpDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		std::wstring defines[] = {
			L"DEFERRED"
		};

		auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
		auto psByteCode = udsdx::CompileShader(m_path, defines, L"PSDeferred", L"ps_6_0");

		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(vsByteCode.data()),
			vsByteCode.size()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()),
			psByteCode->GetBufferSize()
		};

		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(m_deferredPipelineState.GetAddressOf())
		));

		m_deferredPipelineState->SetName((m_path + L" (Deferred)").c_str());
		DebugConsole::Log("\tDeferred shader compiled");
	}

	ID3D12PipelineState* Shader::DefaultPipelineState() const
	{
		return m_defaultPipelineState.Get();
	}

	ID3D12PipelineState* Shader::RiggedPipelineState() const
	{
		return m_riggedPipelineState.Get();
	}

	ID3D12PipelineState* Shader::ShadowPipelineState() const
	{
		return m_shadowPipelineState.Get();
	}

	ID3D12PipelineState* Shader::RiggedShadowPipelineState() const
	{
		return m_riggedShadowPipelineState.Get();
	}

	ID3D12PipelineState* Shader::DeferredPipelineState() const
	{
		return m_deferredPipelineState.Get();
	}
}