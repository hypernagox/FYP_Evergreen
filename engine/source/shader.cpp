#include "pch.h"
#include "shader.h"

namespace udsdx
{
	Shader::Shader(std::wstring_view path) : ResourceObject()
	{
		m_vsByteCode = d3dUtil::CompileShader(path.data(), nullptr, "VS", "vs_5_0");
		m_psByteCode = d3dUtil::CompileShader(path.data(), nullptr, "PS", "ps_5_0");
	}

	void Shader::BuildPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.InputLayout = { Vertex::DescriptionTable, Vertex::DescriptionTableSize };
		psoDesc.pRootSignature = pRootSignature;
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
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1; // m_4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = 0; // m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(m_pipelineState.GetAddressOf())
		));
	}

	ID3D12PipelineState* Shader::PipelineState() const
	{
		return m_pipelineState.Get();
	}
}