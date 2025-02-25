#include "pch.h"
#include "MonsterHPPanel.h"

ComPtr<ID3D12PipelineState> MonsterHPPanel::m_pipelineState;

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

	static float3 gVertexData[4] = 
	{
		float3(-0.5f, -0.5f, 0.0f),
		float3(-0.5f, 0.5f, 0.0f),
		float3(0.5f, -0.5f, 0.0f),
		float3(0.5f, 0.5f, 0.0f)
	};

	static uint gIndexData[6] = 
	{
 		0, 1, 2, 2, 1, 3
	};

    struct PSInput
	{
		float4 Pos : SV_POSITION;
		float4 Color : COLOR;
	};

	PSInput VS(uint vid : SV_VertexID)
	{
		// Extract the camera's right, up, and forward vectors from the view matrix
		float3 right   = normalize(float3(gView._11, gView._21, gView._31)); // First column
		float3 up      = normalize(float3(gView._12, gView._22, gView._32)); // Second column
		float3 forward = normalize(float3(gView._13, gView._23, gView._33)); // Third column

		// Billboard World Matrix: Position stays the same, but it faces the camera
		float4x4 billboardWorld =
		{
			right.x,   right.y,   right.z,   0.0,
			up.x,      up.y,      up.z,      0.0,
			forward.x, forward.y, forward.z, 0.0,
			gWorld._41, gWorld._42, gWorld._43, 1.0 // Keep original position
		};

        float4x4 tLocal = gPrevWorld;
		tLocal._11 = vid < 6 ? tLocal._11 : 1.0f - tLocal._11;
		tLocal._41 = vid < 6 ? (tLocal._11 - 1.0f) * 0.5f : (1.0f - tLocal._11) * 0.5f;
		float4 localPos = float4(gVertexData[gIndexData[vid % 6]], 1.0f);
		float4 worldPos = mul(mul(localPos, tLocal), billboardWorld);

		float4 projPos = mul(worldPos, gViewProj);
		float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

		if (vid < 6)
		{
			float t = tLocal._11;
			float4 color1 = float4(1.0f, 0.0f, 0.0f, 1.0f); // Red (t = 0)
			float4 color2 = float4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow (t = 0.5)
			float4 color3 = float4(0.0f, 1.0f, 0.0f, 1.0f); // Green (t = 1.0)

			if (t <= 0.5f)
			{
				float alpha = t / 0.5f;
				color = lerp(color1, color2, alpha);
			}
			else
			{
				float alpha = (t - 0.5f) / 0.5f;
				color = lerp(color2, color3, alpha);
			}
		}
		PSInput pin;
		pin.Pos = projPos;
		pin.Color = color;
		return pin;
	}

	float4 PS(PSInput pin) : SV_TARGET
	{
		return pin.Color;
	}
)";

MonsterHPPanel::MonsterHPPanel(const std::shared_ptr<SceneObject>& object) : RendererBase(object)
{
	m_castShadow = false;
	m_renderGroup = RenderGroup::Forward;
	if (m_pipelineState == nullptr)
		BuildPipelineState();
}

void MonsterHPPanel::Render(udsdx::RenderParam& param, int instances)
{
	ObjectConstants objectConstants;
	Matrix4x4 worldTransform = XMLoadFloat4x4(&m_transformCache);
	objectConstants.World = worldTransform.Transpose();
	objectConstants.PrevWorld = Matrix4x4::CreateScale(m_hpFraction, 0.1f, 0.1f).Transpose();

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);
	param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	param.CommandList->DrawInstanced(12, instances, 0, 0);
}

void MonsterHPPanel::BuildPipelineState()
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
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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