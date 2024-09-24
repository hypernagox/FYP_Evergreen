#include "pch.h"
#include "shader.h"
#include "debug_console.h"
#include "deferred_renderer.h"

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

		psoDesc.pRootSignature = pRootSignature;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
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

		auto m_psByteCode = d3dUtil::CompileShader(m_path, nullptr, "PS", "ps_5_0");
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
			m_psByteCode->GetBufferSize()
		};

		{
			auto m_vsByteCode = d3dUtil::CompileShader(m_path, nullptr, "VS", "vs_5_0");

			psoDesc.InputLayout = { Vertex::DescriptionTable, Vertex::DescriptionTableSize };
			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
				m_vsByteCode->GetBufferSize()
			};

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_defaultPipelineState.GetAddressOf())
			));

			DebugConsole::Log("\tDefault shader compiled");
		}

		{
			D3D_SHADER_MACRO defines[] =
			{
				"RIGGED", NULL,
				NULL, NULL
			};

			auto m_vsByteCode = d3dUtil::CompileShader(m_path, defines, "VS", "vs_5_0");

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

			DebugConsole::Log("\tRigged shader compiled");
		}

		psoDesc.NumRenderTargets = 0;
		for (UINT i = 0; i < DeferredRenderer::NUM_GBUFFERS; ++i)
		{
			psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}

		{	
			D3D_SHADER_MACRO defines[] =
			{
				"GENERATE_SHADOWS", NULL,
				NULL, NULL
			};

			auto m_vsByteCode = d3dUtil::CompileShader(m_path, defines, "VS", "vs_5_0");
			auto m_psByteCode = d3dUtil::CompileShader(m_path, defines, "ShadowPS", "ps_5_0");

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

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_shadowPipelineState.GetAddressOf())
			));

			DebugConsole::Log("\tShadow shader compiled");
		}

		{
			D3D_SHADER_MACRO defines[] =
			{
				"RIGGED", NULL,
				"GENERATE_SHADOWS", NULL,
				NULL, NULL
			};

			auto m_vsByteCode = d3dUtil::CompileShader(m_path, defines, "VS", "vs_5_0");
			auto m_psByteCode = d3dUtil::CompileShader(m_path, defines, "ShadowPS", "ps_5_0");

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

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_riggedShadowPipelineState.GetAddressOf())
			));

			DebugConsole::Log("\tRigged shadow shader compiled");
		}
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
}