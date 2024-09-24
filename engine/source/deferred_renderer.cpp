#include "pch.h"
#include "deferred_renderer.h"
#include "screen_space_ao.h"
#include "shadow_map.h"
#include "scene.h"

namespace udsdx
{
	constexpr char g_psoRenderResource[] = R"(

		cbuffer cbPerCamera : register(b0)
		{
			float4x4 gView;
			float4x4 gProj;
			float4 gEyePosW;
		}

		cbuffer cbPerShadow : register(b1)
		{
			float4x4 gLightViewProj[4];
			float4x4 gLightViewProjClip[4];
			float4 gShadowDistance;
			float4 gShadowBias;
			float3 gDirLight;
		};

		// Nonnumeric values cannot be added to a cbuffer.
		Texture2D gBuffer1    : register(t0);
		Texture2D gBuffer2    : register(t1);
		Texture2D gBuffer3    : register(t2);
		Texture2D gShadowMap  : register(t3);
		Texture2D gSSAOMap	  : register(t4);
		Texture2D gBufferDSV  : register(t5);

		SamplerState gsamPointClamp : register(s0);
		SamplerState gsamLinearClamp : register(s1);
		SamplerState gsamDepthMap : register(s2);
		SamplerState gsamLinearWrap : register(s3);
		SamplerComparisonState gSamplerShadow : register(s4);

		static const float4x4 gTex = {
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
		};
 
		static const float2 gTexCoords[6] =
		{
			float2(0.0f, 1.0f),
			float2(0.0f, 0.0f),
			float2(1.0f, 0.0f),
			float2(0.0f, 1.0f),
			float2(1.0f, 0.0f),
			float2(1.0f, 1.0f)
		};
 
		struct VertexOut
		{
			float4 PosH : SV_POSITION;
			float2 TexC : TEXCOORD;
		};

		VertexOut VS(uint vid : SV_VertexID)
		{
			VertexOut vout;

			vout.TexC = gTexCoords[vid];

			// Quad covering screen in NDC space.
			vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

			return vout;
		}

		float ShadowValue(float4 posW, float3 normalW, float distanceH)
		{
			uint mipLevel = 0;
			if (distanceH < gShadowDistance[0])
				mipLevel = 0;
			else if (distanceH < gShadowDistance[1])
				mipLevel = 1;
			else if (distanceH < gShadowDistance[2])
				mipLevel = 2;
			else if (distanceH < gShadowDistance[3])
				mipLevel = 3;
			else
				return 1.0f;
			float4 shadowPosH = mul(mul(posW, gLightViewProjClip[mipLevel]), gTex);

			// Complete projection by doing division by w.
			shadowPosH.xy /= shadowPosH.w;
    
			float percentLit = 0.0f;
			if (max(shadowPosH.x, shadowPosH.y) < 1.0f && min(shadowPosH.x, shadowPosH.y) > 0.0f)
			{
				// Depth in NDC space.
				float depth = shadowPosH.z;

				uint width, height, numMips;
				gShadowMap.GetDimensions(0, width, height, numMips);

				// Texel size.
				float dx = 1.0f / (float)width;
				float tanLight = abs(length(cross(normalW, -gDirLight)) / dot(normalW, -gDirLight));
				float bias = tanLight * 2e-5f;

				const float2 offsets[9] = {
					float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
					float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
					float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
				};

				[unroll]
				for (int i = 0; i < 9; ++i)
					percentLit += gShadowMap.SampleCmpLevelZero(gSamplerShadow, shadowPosH.xy + offsets[i], depth - bias).r;
			}
			else
			{
				percentLit = 9.0f;
			}
    
			return percentLit / 9.0f;
		}
 
		float4 PS(VertexOut pin) : SV_Target
		{
			float4 gBuffer1Color = gBuffer1.Sample(gsamLinearClamp, pin.TexC);
			float3 normalV = gBuffer2.Sample(gsamLinearClamp, pin.TexC).xyz;
			float3 normalW = normalize(mul(normalV, transpose((float3x3)gView)));
			float4 PosW = float4(gBuffer3.Sample(gsamLinearClamp, pin.TexC).xyz, 1.0f);
			float distanceH = length(PosW.xyz - gEyePosW.xyz);

			float diffuse = pow(saturate(dot(normalW, -gDirLight) * 1.1f - 0.1f), 0.3f);
			float shadowValue = ShadowValue(PosW, normalW, distanceH);
			float AOFactor = gSSAOMap.Sample(gsamLinearClamp, pin.TexC).r;
			gBuffer1Color.rgb = (gBuffer1Color.rgb - (1.0f - min(diffuse, shadowValue)) * 0.75f) * AOFactor;
			return gBuffer1Color;
		}
	)";

    DeferredRenderer::DeferredRenderer(ID3D12Device* device)
    {
		m_device = device;

		BuildRootSignature();
    }

    DeferredRenderer::~DeferredRenderer()
	{
	}

	void DeferredRenderer::OnResize(UINT newWidth, UINT newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;

		BuildResources();
	}

	void DeferredRenderer::BuildDescriptors(DescriptorParam& descriptorParam)
    {
        for (UINT i = 0; i < NUM_GBUFFERS; ++i)
        {
            m_gBuffersCpuSrv[i] = descriptorParam.SrvCpuHandle;
            m_gBuffersGpuSrv[i] = descriptorParam.SrvGpuHandle;
			m_gBuffersCpuRtv[i] = descriptorParam.RtvCpuHandle;

			descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
			descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
			descriptorParam.RtvCpuHandle.Offset(1, descriptorParam.RtvDescriptorSize);
        }

		m_depthBufferCpuSrv = descriptorParam.SrvCpuHandle;
		m_depthBufferGpuSrv = descriptorParam.SrvGpuHandle;
		m_depthBufferCpuDsv = descriptorParam.DsvCpuHandle;

		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.DsvCpuHandle.Offset(1, descriptorParam.DsvDescriptorSize);
    }

	void DeferredRenderer::BuildRootSignature()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC samplerShadow(
			4,
			D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			0.0f,
			16,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

		std::array<CD3DX12_STATIC_SAMPLER_DESC, 5> staticSamplers =
		{
			pointClamp, linearClamp, depthMapSam, linearWrap, samplerShadow
		};

		CD3DX12_DESCRIPTOR_RANGE texTable1;
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

		CD3DX12_DESCRIPTOR_RANGE texTable2;
		texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);

		CD3DX12_DESCRIPTOR_RANGE texTable3;
		texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);

		CD3DX12_DESCRIPTOR_RANGE texTable4;
		texTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);

		CD3DX12_ROOT_PARAMETER slotRootParameter[6];

		// Perfomance TIP: Order from most frequent to least frequent.
		slotRootParameter[0].InitAsConstants(sizeof(CameraConstants) / 4, 0);
		slotRootParameter[1].InitAsConstantBufferView(1);
		slotRootParameter[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[3].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[4].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[5].InitAsDescriptorTable(1, &texTable4, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
			static_cast<UINT>(staticSamplers.size()), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(m_device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(m_renderRootSignature.GetAddressOf())));
	}

    void DeferredRenderer::RebuildDescriptors()
	{
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        // Create the render target view
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        for (UINT i = 0; i < NUM_GBUFFERS; ++i)
		{
			srvDesc.Format = GBUFFER_FORMATS[i];
            rtvDesc.Format = GBUFFER_FORMATS[i];
			m_device->CreateShaderResourceView(m_gBuffers[i].Get(), &srvDesc, m_gBuffersCpuSrv[i]);
            m_device->CreateRenderTargetView(m_gBuffers[i].Get(), &rtvDesc, m_gBuffersCpuRtv[i]);
		}

		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		m_device->CreateShaderResourceView(m_depthBuffer.Get(), &srvDesc, m_depthBufferCpuSrv);

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DEPTH_FORMAT;
		dsvDesc.Texture2D.MipSlice = 0;
		m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, m_depthBufferCpuDsv);
	}

    void DeferredRenderer::BuildResources()
    {
		D3D12_RESOURCE_DESC texDesc = {};
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        for (UINT i = 0; i < NUM_GBUFFERS; ++i)
		{
            m_gBuffers[i].Reset();

			texDesc.Format = GBUFFER_FORMATS[i];
			CD3DX12_CLEAR_VALUE clearValue(GBUFFER_FORMATS[i], clearColor);

            ThrowIfFailed(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&clearValue,
				IID_PPV_ARGS(&m_gBuffers[i])));
		}

        m_depthBuffer.Reset();

		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;			// alignment size of the resource. 0 means use default alignment.
		depthStencilDesc.Width = m_width;			// size of the width.
		depthStencilDesc.Height = m_height;		// size of the height.
		depthStencilDesc.DepthOrArraySize = 1;	// size of the depth.
		depthStencilDesc.MipLevels = 1;			// number of mip levels.
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;				// layout of the texture to be used in the pipeline.
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// resource is used as a depth-stencil buffer.

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(&m_depthBuffer)
		));
    }

	void DeferredRenderer::BuildPipelineStateObjects()
	{
		// Build the normal PSO
		auto vsByteCode = d3dUtil::CompileShaderFromMemory(g_psoRenderResource, nullptr, "VS", "vs_5_0");
		auto psByteCode = d3dUtil::CompileShaderFromMemory(g_psoRenderResource, nullptr, "PS", "ps_5_0");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.InputLayout.pInputElementDescs = Vertex::DescriptionTable;
		psoDesc.InputLayout.NumElements = Vertex::DescriptionTableSize;
		psoDesc.pRootSignature = m_renderRootSignature.Get();
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
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(m_renderPipelineState.GetAddressOf())
		));
	}

	void DeferredRenderer::ClearRenderTargets(ID3D12GraphicsCommandList* commandList)
	{
		const float clearValue[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (UINT i = 0; i < NUM_GBUFFERS; ++i)
		{
			commandList->ClearRenderTargetView(m_gBuffersCpuRtv[i], clearValue, 0, nullptr);
		}
		commandList->ClearDepthStencilView(m_depthBufferCpuDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

    void DeferredRenderer::SetRenderTargets(ID3D12GraphicsCommandList* commandList)
    {
		commandList->OMSetRenderTargets(NUM_GBUFFERS, m_gBuffersCpuRtv.data(), true, &m_depthBufferCpuDsv);
    }

	void DeferredRenderer::PassBufferPreparation(RenderParam& renderParam)
	{
		ID3D12GraphicsCommandList* pCommandList = renderParam.CommandList;

		for (UINT i = 0; i < NUM_GBUFFERS; ++i)
		{
			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_gBuffers[i].Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		pCommandList->SetGraphicsRootSignature(renderParam.RootSignature);

		ClearRenderTargets(pCommandList);
		pCommandList->OMSetRenderTargets(NUM_GBUFFERS, m_gBuffersCpuRtv.data(), true, &m_depthBufferCpuDsv);

		pCommandList->RSSetViewports(1, &renderParam.Viewport);
		pCommandList->RSSetScissorRects(1, &renderParam.ScissorRect);

		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void DeferredRenderer::PassBufferPostProcess(RenderParam& renderParam)
	{
		ID3D12GraphicsCommandList* pCommandList = renderParam.CommandList;

		for (UINT i = 0; i < NUM_GBUFFERS; ++i)
		{
			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_gBuffers[i].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
		}
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void DeferredRenderer::PassRender(RenderParam& renderParam)
	{
		ID3D12GraphicsCommandList* pCommandList = renderParam.CommandList;

		pCommandList->SetGraphicsRootSignature(m_renderRootSignature.Get());

		pCommandList->OMSetRenderTargets(1, &renderParam.RenderTargetView, true, nullptr);

		pCommandList->RSSetViewports(1, &renderParam.Viewport);
		pCommandList->RSSetScissorRects(1, &renderParam.ScissorRect);

		pCommandList->IASetVertexBuffers(0, 0, nullptr);
		pCommandList->IASetIndexBuffer(nullptr);
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCommandList->SetPipelineState(m_renderPipelineState.Get());

		pCommandList->SetGraphicsRootConstantBufferView(1, renderParam.RenderShadowMap->GetConstantBuffer(renderParam.FrameResourceIndex));
		pCommandList->SetGraphicsRootDescriptorTable(2, m_gBuffersGpuSrv[0]);
		pCommandList->SetGraphicsRootDescriptorTable(3, renderParam.RenderShadowMap->GetSrvGpu());
		pCommandList->SetGraphicsRootDescriptorTable(4, renderParam.RenderScreenSpaceAO->GetAmbientMapGpuSrv());
		pCommandList->SetGraphicsRootDescriptorTable(5, m_depthBufferGpuSrv);

		pCommandList->DrawInstanced(6, 1, 0, 0);
	}
}