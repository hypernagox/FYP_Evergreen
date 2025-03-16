#include "pch.h"
#include "gui_image.h"
#include "scene.h"
#include "core.h"
#include "shader_compile.h"
#include "transform.h"
#include "texture.h"

namespace udsdx
{
	static constexpr char* g_psoResource = R"(
		cbuffer cbPerObject : register(b0)
		{
			float4x4 gWorld;
			float4x4 gPrevWorld;
		};

		cbuffer cbPerFrame : register(b4)
		{
			float gTime;
			float gDeltaTime;
			float gMotionBlurFactor;
			float gMotionBlurRadius;
		};

		Texture2D gMainTex : register(t0);
		SamplerState gSampler : register(s0);

        struct VertexOut
		{
			float4 pos : SV_POSITION;
			float2 uv : TEXCOORD0;
		};

		static float2 gVertexData[4] = { float2(-0.5f, -0.5f), float2(0.5f, -0.5f), float2(-0.5f, 0.5f), float2(0.5f, 0.5f) };
		static float2 gUVData[4] = { float2(0.0f, 1.0f), float2(1.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f) };
		static uint gIndexData[6] = { 0, 2, 1, 3, 1, 2 };

		VertexOut VS(uint id : SV_VertexID)
		{
			VertexOut output;
			output.pos = mul(float4(gVertexData[gIndexData[id]], 0.0f, 1.0f), gWorld);
			output.uv = gUVData[gIndexData[id]];
			return output;
		}

		float4 PS(VertexOut input) : SV_TARGET
		{
			return gMainTex.Sample(gSampler, input.uv);
		}
	)";

	ComPtr<ID3D12PipelineState> GUIImage::g_pipelineState = nullptr;

	ID3D12PipelineState* GUIImage::GetPipelineState()
	{
		if (g_pipelineState == nullptr)
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
			psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable = false;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleDesc.Quality = 0;
			psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

			psoDesc.VS.pShaderBytecode = reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),
			psoDesc.VS.BytecodeLength = vsByteCode->GetBufferSize();
			psoDesc.PS.pShaderBytecode = reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer());
			psoDesc.PS.BytecodeLength = psByteCode->GetBufferSize();

			psoDesc.InputLayout.NumElements = 0;
			psoDesc.InputLayout.pInputElementDescs = nullptr;

			ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));
		}

		return g_pipelineState.Get();
	}

	GUIImage::GUIImage(const std::shared_ptr<SceneObject>& object) : Component(object)
	{
	}

	void GUIImage::PostUpdate(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderGUIObject(this);
	}

	void GUIImage::Render(RenderParam& param, int instances)
	{
		const float height = 1080.0f;
		const float width = height * param.AspectRatio;
		Matrix4x4 world = Matrix4x4::CreateScale(m_size.x, m_size.y, 1.0f) * GetTransform()->GetWorldSRTMatrix() * Matrix4x4::CreateScale(2.0f / width, 2.0f / height, 1.0f);
		param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, 16, &world.Transpose(), 0);
		if (m_texture != nullptr)
		{
			param.CommandList->SetGraphicsRootDescriptorTable(RootParam::MainTexSRV, m_texture->GetSrvGpu());
		}
		param.CommandList->DrawInstanced(6, instances, 0, 0);
	}

	void GUIImage::SetTexture(Texture* value, bool setImageSize)
	{
		m_texture = value;
		if (setImageSize && m_texture)
		{
			m_size = Vector2(static_cast<float>(m_texture->GetWidth()), static_cast<float>(m_texture->GetHeight()));
		}
	}
}