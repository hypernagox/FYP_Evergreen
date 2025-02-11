#include "pch.h"
#include "motion_blur.h"
#include "deferred_renderer.h"
#include "camera.h"

namespace udsdx
{
	constexpr char g_psoTileMax[] = R"(
		Texture2D gSource : register(t0);
		RWTexture2D<float2> gDestination : register(u0);

		[numthreads(16, 16, 1)]
		void CS(int3 id : SV_DispatchThreadID)
		{
			float2 maxSample = 0.0f;
			for (int y = 0; y < MAX_BLUR_RADIUS; ++y)
			{
				for (int x = 0; x < MAX_BLUR_RADIUS; ++x)
				{
					float2 sample = gSource.Load(int3(id.xy * MAX_BLUR_RADIUS + int2(x, y), 0));
					if (dot(sample, sample) > dot(maxSample, maxSample))
					{
						maxSample = sample;
					}
				}
			}
			gDestination[id.xy] = maxSample;
		}
	)";

	constexpr char g_psoNeighborMax[] = R"(
		Texture2D gSource : register(t0);
		RWTexture2D<float2> gDestination : register(u0);

		[numthreads(16, 16, 1)]
		void CS(int3 id : SV_DispatchThreadID)
		{
			float2 maxSample = 0.0f;
			[unroll]
			for (int y = -1; y <= 1; ++y)
			{
				[unroll]
				for (int x = -1; x <= 1; ++x)
				{
					float2 sample = gSource.Load(int3(id.xy + int2(x, y), 0));
					if (dot(sample, sample) > dot(maxSample, maxSample))
					{
						maxSample = sample;
					}
				}
			}
			gDestination[id.xy] = maxSample;
		}
	)";

	constexpr char g_psoPass[] = R"(
		cbuffer cbPerCamera : register(b0)
		{
			float4x4 gView;
			float4x4 gProj;
			float4x4 gViewProj;
			float4x4 gViewInverse;
			float4x4 gProjInverse;
			float4x4 gViewProjInverse;
			float4x4 gPrevViewProj;
			float4 gEyePosW;
			float2 gRenderTargetSize;
		};

		Texture2D gSource : register(t0);
        Texture2D gMotion : register(t1);
		Texture2D gDepth : register(t2);
		Texture2D gNeighborMax : register(t3);
		RWTexture2D<float4> gDestination : register(u0);
		
		static const uint gSampleCount = 16;

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

		float Cone(float x, float r)
		{
			return saturate(1.0f - abs(x) / r);
		}

		float Cylinder(float x, float r)
		{
			return 1.0f - smoothstep(r * 0.95f, r * 1.05f, abs(x));
		}

		float SoftDepthComp(float lhs, float rhs)
		{
			const float ext = 1e-3f;
			return saturate(1.0f - (lhs - rhs) / ext);
		}

		[numthreads(16, 16, 1)]
		void CS(int3 id : SV_DispatchThreadID)
		{
			float2 vn = gNeighborMax.Load(id / MAX_BLUR_RADIUS) * MAX_BLUR_RADIUS;
			vn.y = -vn.y;

			if (length(vn) < 0.5f)
			{
				gDestination[id.xy] = gSource.Load(id);
				return;
			}

			float2 vx = gMotion.Load(id) * MAX_BLUR_RADIUS;
			vx.y = -vx.y;

			float weight = 1.0f / max(length(vx), 1.0f);
			float4 color = gSource.Load(id) * weight;

			float depthSrc = NdcDepthToViewDepth(gDepth.Load(id).r);
			float bias = rand3(float3(id.xy / gRenderTargetSize, 0.0f)).x;

			[unroll]
			for (uint i = 0; i < gSampleCount; ++i)
			{
				if (i == (gSampleCount - 1) / 2)
				{
					continue;
				}

				float t = lerp(-1.0f, 1.0f, (i + bias + 1.0f) / (gSampleCount + 1.0f));
				int3 idDest = int3(id.xy + vn * t, 0);
				float depthDst = NdcDepthToViewDepth(gDepth.Load(idDest).r);

				float d = length(vn) * t;
				float2 vy = gMotion.Load(idDest) * MAX_BLUR_RADIUS;
				vy.y = -vy.y;
                float y =	SoftDepthComp(depthDst, depthSrc) * Cone(d, length(vy)) +
							SoftDepthComp(depthSrc, depthDst) * Cone(d, length(vx)) +
							Cylinder(d, length(vy)) * Cylinder(d, length(vx)) * 2.0f;

				weight += y;
				color += gSource.Load(idDest) * y; 
			}

			gDestination[id.xy] = color / weight;
		}
	)";

	MotionBlur::MotionBlur(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		m_device = device;
		BuildRootSignature();
	}

	void MotionBlur::Pass(RenderParam& param, D3D12_GPU_VIRTUAL_ADDRESS cbvGpu)
	{
		auto pCommandList = param.CommandList;

		UINT computeWidth = (m_width + MaxBlurRadius - 1) / MaxBlurRadius;
		UINT computeHeight = (m_height + MaxBlurRadius - 1) / MaxBlurRadius;

		// Compute the dominant motion vectors per k * k area
		pCommandList->SetComputeRootSignature(m_computeRootSignature.Get());

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_tileMaxBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		pCommandList->SetPipelineState(m_tileMaxPso.Get());
		pCommandList->SetComputeRootDescriptorTable(0, param.Renderer->GetGBufferSrv(2));
		pCommandList->SetComputeRootDescriptorTable(1, m_tileMaxGpuUav);

		pCommandList->Dispatch((computeWidth + 15) / 16, (computeHeight + 15) / 16, 1);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_tileMaxBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_neighborMaxBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		pCommandList->SetPipelineState(m_neighborMaxPso.Get());
		pCommandList->SetComputeRootDescriptorTable(0, m_tileMaxGpuSrv);
		pCommandList->SetComputeRootDescriptorTable(1, m_neighborMaxGpuUav);

		pCommandList->Dispatch((computeWidth + 15) / 16, (computeHeight + 15) / 16, 1);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_neighborMaxBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		// Copy the render target resource to the source buffer
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

		pCommandList->CopyResource(m_sourceBuffer.Get(), param.RenderTargetResource);


		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		// Set the render target
		pCommandList->SetComputeRootSignature(m_rootSignature.Get());
		pCommandList->SetPipelineState(m_pso.Get());

		pCommandList->SetComputeRootConstantBufferView(0, cbvGpu);
		pCommandList->SetComputeRootDescriptorTable(1, m_sourceGpuSrv);
		pCommandList->SetComputeRootDescriptorTable(2, param.Renderer->GetGBufferSrv(2));
		pCommandList->SetComputeRootDescriptorTable(3, param.Renderer->GetDepthBufferSrv());
		pCommandList->SetComputeRootDescriptorTable(4, m_neighborMaxGpuSrv);
		pCommandList->SetComputeRootDescriptorTable(5, m_destinationGpuUav);

		pCommandList->Dispatch((m_width + 15) / 16, (m_height + 15) / 16, 1);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_destinationBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

		pCommandList->CopyResource(param.RenderTargetResource, m_destinationBuffer.Get());

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_destinationBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}

	void MotionBlur::OnResize(UINT newWidth, UINT newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;

		BuildResources();
	}

	void MotionBlur::BuildResources()
	{
		m_tileMaxBuffer.Reset();
		m_neighborMaxBuffer.Reset();
		m_sourceBuffer.Reset();

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = (m_width + MaxBlurRadius - 1) / MaxBlurRadius;
		texDesc.Height = (m_height + MaxBlurRadius - 1) / MaxBlurRadius;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R16G16_SNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_tileMaxBuffer.GetAddressOf())));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_neighborMaxBuffer.GetAddressOf())));

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
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_sourceBuffer.GetAddressOf())));

		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(m_destinationBuffer.GetAddressOf())));
	}

	void MotionBlur::BuildDescriptors(DescriptorParam& descriptorParam)
	{
		m_tileMaxCpuSrv = descriptorParam.SrvCpuHandle;
		m_tileMaxGpuSrv = descriptorParam.SrvGpuHandle;
		m_tileMaxCpuUav = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_tileMaxGpuUav = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_neighborMaxCpuSrv = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_neighborMaxGpuSrv = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_neighborMaxCpuUav = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_neighborMaxGpuUav = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_sourceCpuSrv = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_sourceGpuSrv = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_destinationCpuUav = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_destinationGpuUav = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		RebuildDescriptors();
	}

	void MotionBlur::RebuildDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R16G16_SNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		m_device->CreateShaderResourceView(m_tileMaxBuffer.Get(), &srvDesc, m_tileMaxCpuSrv);
		m_device->CreateShaderResourceView(m_neighborMaxBuffer.Get(), &srvDesc, m_neighborMaxCpuSrv);

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		m_device->CreateShaderResourceView(m_sourceBuffer.Get(), &srvDesc, m_sourceCpuSrv);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

		uavDesc.Format = DXGI_FORMAT_R16G16_SNORM;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		m_device->CreateUnorderedAccessView(m_tileMaxBuffer.Get(), nullptr, &uavDesc, m_tileMaxCpuUav);
		m_device->CreateUnorderedAccessView(m_neighborMaxBuffer.Get(), nullptr, &uavDesc, m_neighborMaxCpuUav);

		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		m_device->CreateUnorderedAccessView(m_destinationBuffer.Get(), nullptr, &uavDesc, m_destinationCpuUav);
	}

	void MotionBlur::BuildRootSignature()
	{
		{
			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable2;
			texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			CD3DX12_DESCRIPTOR_RANGE texTable3;
			texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

			CD3DX12_DESCRIPTOR_RANGE texTable4;
			texTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);

			CD3DX12_DESCRIPTOR_RANGE texTable5;
			texTable5.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			CD3DX12_ROOT_PARAMETER slotRootParameter[6]{};

			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable1);
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable2);
			slotRootParameter[3].InitAsDescriptorTable(1, &texTable3);
			slotRootParameter[4].InitAsDescriptorTable(1, &texTable4);
			slotRootParameter[5].InitAsDescriptorTable(1, &texTable5);

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

		{
			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable2;
			texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			CD3DX12_ROOT_PARAMETER slotRootParameter[2]{};

			slotRootParameter[0].InitAsDescriptorTable(1, &texTable1);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable2);

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
				IID_PPV_ARGS(m_computeRootSignature.GetAddressOf())));
		}
	}

	void MotionBlur::BuildPipelineState()
	{
		auto itos = [](int i) -> std::string {
			std::stringstream ss;
			ss << i;
			return ss.str();
			};

		std::string sMAX_BLUR_RADIUS = itos(MaxBlurRadius);

		D3D_SHADER_MACRO defines[] =
		{
			"MAX_BLUR_RADIUS", sMAX_BLUR_RADIUS.c_str(),
			nullptr, nullptr
		};

		{
			auto csByteCode = d3dUtil::CompileShaderFromMemory(g_psoTileMax, defines, "CS", "cs_5_0");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			psoDesc.pRootSignature = m_computeRootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode->GetBufferPointer()),
				csByteCode->GetBufferSize()
			};

			ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_tileMaxPso.GetAddressOf())));
		}

		{
			auto csByteCode = d3dUtil::CompileShaderFromMemory(g_psoNeighborMax, defines, "CS", "cs_5_0");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			psoDesc.pRootSignature = m_computeRootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode->GetBufferPointer()),
				csByteCode->GetBufferSize()
			};

			ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_neighborMaxPso.GetAddressOf())));
		}

		{
			auto csByteCode = d3dUtil::CompileShaderFromMemory(g_psoPass, defines, "CS", "cs_5_0");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode->GetBufferPointer()),
				csByteCode->GetBufferSize()
			};

			ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_pso.GetAddressOf())));
		}
	}
}