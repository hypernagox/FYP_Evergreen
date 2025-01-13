#include "pch.h"
#include "motion_blur.h"
#include "deferred_renderer.h"
#include "camera.h"

namespace udsdx
{
	constexpr char g_psoPass[] = R"(
		cbuffer cbMotion : register(b0)
		{
			float4x4 gProj;
			float gDeltaTime;
		};

		Texture2D gSource : register(t0);
        Texture2D gMotion : register(t1);
		Texture2D gDepth : register(t2);
		SamplerState gsamPointClamp : register(s0);
 
		static const float2 gTexCoords[6] =
		{
			float2(0.0f, 1.0f),
			float2(0.0f, 0.0f),
			float2(1.0f, 0.0f),
			float2(0.0f, 1.0f),
			float2(1.0f, 0.0f),
			float2(1.0f, 1.0f)
		};
		
		static const uint gSampleCount = 16;
		static const float gBlurThreshold = 0.0001f;
		static const float gBlurTime = 0.01f;

		static const float2x2 gTex = { 0.5f, 0.0f, 0.0f, -0.5f, };
 
		struct VertexOut
		{
			float4 PosH : SV_POSITION;
			float2 TexC : TEXCOORD;
		};

		// discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3
		float3 rand3(float3 c) {
			float j = 4096.0f * sin(dot(c, float3(17.0f, 59.4f, 15.0f)));
			float3 r;
			r.z = frac(512.0f * j);
			j *= 0.125f;
			r.x = frac(512.0f * j);
			j *= 0.125f;
			r.y = frac(512.0f * j);
			return r - 0.5f;
		}

		float NdcDepthToViewDepth(float z_ndc)
		{
			float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
			return viewZ;
		}

		float softDepthComp(float lhs, float rhs)
		{
			const float ext = 1e-3f;
			return saturate(1.0f - (lhs - rhs) / ext);
		}

		VertexOut VS(uint vid : SV_VertexID)
		{
			VertexOut vout;

			vout.TexC = gTexCoords[vid];

			// Quad covering screen in NDC space.
			vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

			return vout;
		}

		float4 PS(VertexOut pin) : SV_Target
		{
			float2 mv = mul(gMotion.Sample(gsamPointClamp, pin.TexC).rg, gTex);
			mv *= gBlurTime / gDeltaTime;

			if (length(mv) < gBlurThreshold)
			{
				return gSource.Sample(gsamPointClamp, pin.TexC);
			}

			float4 color = 0.0f;
			float weightSum = 0.0f;
			float depthSrc = gDepth.Sample(gsamPointClamp, pin.TexC).r;
			float bias = rand3(float3(pin.TexC, 0.0f)).x + 0.5f;
			[unroll]
			for (uint i = 0; i < gSampleCount; ++i)
			{
				float2 offset = mv * (i + bias) / float(gSampleCount);
				float depthDst = gDepth.Sample(gsamPointClamp, pin.TexC + offset).r;

				float weight = softDepthComp(depthSrc, depthDst);
				color += gSource.Sample(gsamPointClamp, pin.TexC + offset) * weight;
				weightSum += weight;
			}

			if (weightSum > 0.0f)
				return color / weightSum;
			return gSource.Sample(gsamPointClamp, pin.TexC);
		}
	)";

	MotionBlur::MotionBlur(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		m_device = device;

		BuildRootSignature();
	}

	void MotionBlur::Pass(RenderParam& param, const Camera* camera)
	{
		auto pCommandList = param.CommandList;

		// Copy the render target resource to the source buffer
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

		pCommandList->CopyResource(m_sourceBuffer.Get(), param.RenderTargetResource);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		// Set the render target
		pCommandList->RSSetViewports(1, &param.Viewport);
		pCommandList->RSSetScissorRects(1, &param.ScissorRect);

		pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());
		pCommandList->SetPipelineState(m_pso.Get());
		
		pCommandList->IASetVertexBuffers(0, 0, nullptr);
		pCommandList->IASetIndexBuffer(nullptr);

		pCommandList->SetGraphicsRoot32BitConstants(0, 16, &camera->GetProjMatrix(param.AspectRatio).Transpose(), 0);
		pCommandList->SetGraphicsRoot32BitConstants(0, 1, &param.Time.deltaTime, 16);
		pCommandList->SetGraphicsRootDescriptorTable(1, m_sourceGpuSrv);
		pCommandList->SetGraphicsRootDescriptorTable(2, param.Renderer->GetGBufferSrv(2));
		pCommandList->SetGraphicsRootDescriptorTable(3, param.Renderer->GetDepthBufferSrv());

		pCommandList->OMSetRenderTargets(1, &param.RenderTargetView, true, nullptr);

		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->DrawInstanced(6, 1, 0, 0);
	}

	void MotionBlur::OnResize(UINT newWidth, UINT newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;

		BuildResources();
	}

	void MotionBlur::BuildResources()
	{
		m_sourceBuffer.Reset();

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(m_sourceBuffer.GetAddressOf())));
	}

	void MotionBlur::BuildDescriptors(DescriptorParam& descriptorParam)
	{
		m_sourceCpuSrv = descriptorParam.SrvCpuHandle;
		m_sourceGpuSrv = descriptorParam.SrvGpuHandle;

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		RebuildDescriptors();
	}

	void MotionBlur::RebuildDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		m_device->CreateShaderResourceView(m_sourceBuffer.Get(), &srvDesc, m_sourceCpuSrv);
	}

	void MotionBlur::BuildRootSignature()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		CD3DX12_DESCRIPTOR_RANGE texTable1;
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE texTable2;
		texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE texTable3;
		texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

		CD3DX12_ROOT_PARAMETER slotRootParameter[4]{};

		slotRootParameter[0].InitAsConstants(17, 0);
		slotRootParameter[1].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[2].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[3].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, 1, &pointClamp, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(m_device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
	}

	void MotionBlur::BuildPipelineState()
	{
		auto vsByteCode = d3dUtil::CompileShaderFromMemory(g_psoPass, nullptr, "VS", "vs_5_0");
		auto psByteCode = d3dUtil::CompileShaderFromMemory(g_psoPass, nullptr, "PS", "ps_5_0");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		// There's no vertex input in this case
		psoDesc.InputLayout.pInputElementDescs = nullptr;
		psoDesc.InputLayout.NumElements = 0;
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),
			vsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()),
			psByteCode->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(m_pso.GetAddressOf())
		));
	}
}