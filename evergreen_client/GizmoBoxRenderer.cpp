#include "pch.h"
#include "GizmoBoxRenderer.h"

static constexpr char g_psoResource[] = R"(
	cbuffer cbPerObject : register(b0)
	{
		float4x4 gWorld;
		float4x4 gPrevWorld;
	};

	cbuffer cbPerCamera : register(b1)
	{
		float4x4 gView;
		float4x4 gProj;
		float4x4 gViewProj;
		float4x4 gViewInverse;
		float4x4 gProjInverse;
		float4x4 gViewProjInverse;
		float4x4 gPrevViewProj;
		float4 gEyePosW;
	};

	static float3 gVertexData[8] = 
	{
		float3(-0.5f, -0.5f, -0.5f),
        float3(-0.5f, -0.5f, 0.5f),
		float3(-0.5f, 0.5f, -0.5f),
		float3(-0.5f, 0.5f, 0.5f),
		float3(0.5f, -0.5f, -0.5f),
		float3(0.5f, -0.5f, 0.5f),
		float3(0.5f, 0.5f, -0.5f),
		float3(0.5f, 0.5f, 0.5f)
	};

	static uint gIndexData[24] = 
	{
		0, 1, 1, 3, 3, 2, 2, 0,
		4, 5, 5, 7, 7, 6, 6, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	float4 VS(uint vid : SV_VertexID) : SV_POSITION
	{
		float4 worldPos = mul(float4(gVertexData[gIndexData[vid]], 1.0f), gWorld);
		float4 projPos = mul(worldPos, gViewProj);
		return projPos;
	}

	float4 PS(float4 pos : SV_POSITION) : SV_TARGET
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
)";

using namespace udsdx;

GizmoBoxRenderer::GizmoBoxRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
{
	m_castShadow = false;
	m_renderGroup = RenderGroup::Forward;
	BuildPipelineState();
}

void GizmoBoxRenderer::Render(RenderParam& param, int instances)
{
	ObjectConstants objectConstants;
	Matrix4x4 worldTransform = XMMatrixScaling(m_size.x, m_size.y, m_size.z) * XMMatrixTranslation(m_offset.x, m_offset.y, m_offset.z) * XMLoadFloat4x4(&m_transformCache);
	objectConstants.World = worldTransform.Transpose();
	objectConstants.PrevWorld = m_prevTransformCache.Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
	param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	param.CommandList->DrawInstanced(24, instances, 0, 0);
}

void GizmoBoxRenderer::BuildPipelineState()
{
	ID3D12RootSignature* rootSignature = INSTANCE(Core)->GetRootSignature();
	ID3D12Device* device = INSTANCE(Core)->GetDevice();

	static ComPtr<ID3DBlob> vsByteCode = nullptr;
	static ComPtr<ID3DBlob> psByteCode = nullptr;

	if (vsByteCode == nullptr || psByteCode == nullptr)
	{
		vsByteCode = d3dUtil::CompileShaderFromMemory(g_psoResource, nullptr, "VS", "vs_5_0");
		psByteCode = d3dUtil::CompileShaderFromMemory(g_psoResource, nullptr, "PS", "ps_5_0");
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.pRootSignature = rootSignature;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	psoDesc.VS.pShaderBytecode = reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),
	psoDesc.VS.BytecodeLength = vsByteCode->GetBufferSize();
	psoDesc.PS.pShaderBytecode = reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer());
	psoDesc.PS.BytecodeLength = psByteCode->GetBufferSize();

	psoDesc.InputLayout.NumElements = 0;
	psoDesc.InputLayout.pInputElementDescs = nullptr;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}