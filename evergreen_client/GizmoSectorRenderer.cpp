#include "pch.h"
#include "GizmoSectorRenderer.h"

using namespace udsdx;

static constexpr char g_psoResource[] = R"(
	cbuffer cbPerObject : register(b0)
	{
		float4x4 gWorld;
		float    gRadian;
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

	static uint gIndexData[60] = 
	{
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 0,
        10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 10,
        0, 10, 1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 6, 16, 7, 17, 8, 18, 9, 19
	};

	float4 VS(uint vid : SV_VertexID) : SV_POSITION
	{
        uint index = gIndexData[vid];
        float4 position = float4(0.0f, 0.0f, 0.0f, 1.0f);
        if (index >= 10)
            position.y = 1.0f;
        if (index % 10 != 0)
        {
            float angle = (int(index % 10) - 5) * 0.125f * gRadian;
            position.x = sin(angle);
            position.z = cos(angle);
        }
		float4 worldPos = mul(position, gWorld);
		float4 projPos = mul(worldPos, gViewProj);
		return projPos;
	}

	float4 PS(float4 pos : SV_POSITION) : SV_TARGET
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
)";

GizmoSectorRenderer::GizmoSectorRenderer(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
{
	m_castShadow = false;
	m_renderGroup = RenderGroup::Forward;
	BuildPipelineState();
}

void GizmoSectorRenderer::Render(udsdx::RenderParam& param, int instances)
{
	ObjectConstants objectConstants;
	Matrix4x4 worldTransform = XMMatrixScaling(m_radius, 1.0f, m_radius) * XMLoadFloat4x4(&m_transformCache);
	objectConstants.World = worldTransform.Transpose();
	objectConstants.PrevWorld.m[0][0] = m_angle * DEG2RAD;

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
	param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	param.CommandList->DrawInstanced(60, instances, 0, 0);
}

void GizmoSectorRenderer::BuildPipelineState()
{
	ID3D12RootSignature* rootSignature = INSTANCE(Core)->GetRootSignature();
	ID3D12Device* device = INSTANCE(Core)->GetDevice();

	static ComPtr<IDxcBlob> vsByteCode = nullptr;
	static ComPtr<IDxcBlob> psByteCode = nullptr;

	if (vsByteCode == nullptr || psByteCode == nullptr)
	{
		vsByteCode = udsdx::CompileShaderFromMemory(g_psoResource, {}, L"VS", L"vs_6_0");
		psByteCode = udsdx::CompileShaderFromMemory(g_psoResource, {}, L"PS", L"ps_6_0");
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
