#include "pch.h"
#include "screen_space_ao.h"
#include "deferred_renderer.h"
#include "shader_compile.h"
#include "frame_resource.h"
#include "scene_object.h"
#include "transform.h"
#include "camera.h"
#include "scene.h"

namespace udsdx
{
	ScreenSpaceAO::ScreenSpaceAO(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float ssaoMapScale)
	{
		m_device = device;
		m_ssaoMapScale = ssaoMapScale;
		m_width = 0;
		m_height = 0;

		BuildOffsetVectors();
		BuildBlurWeights();
		BuildRootSignature(device);

		for (auto& buffer : m_constantBuffers)
		{
			buffer = std::make_unique<UploadBuffer<SSAOConstants>>(device, 1, true);
		}
		m_blurConstantBuffer = std::make_unique<UploadBuffer<BlurConstants>>(device, 1, true);

		UpdateBlurConstants();
	}

	void ScreenSpaceAO::UpdateSSAOConstants(RenderParam& param, Camera* pCamera)
	{
		SSAOConstants ssaoCB;

		XMMATRIX P = pCamera->GetProjMatrix(param.AspectRatio);

		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		XMStoreFloat4x4(&ssaoCB.Proj, XMMatrixTranspose(P));
		XMStoreFloat4x4(&ssaoCB.InvProj, XMMatrixTranspose(XMMatrixInverse(&XMMatrixDeterminant(P), P)));
		XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

		memcpy(ssaoCB.OffsetVectors, m_offsets, sizeof(m_offsets));

		// Coordinates given in view space.
		ssaoCB.OcclusionRadius = 0.25f;
		ssaoCB.OcclusionFadeStart = 0.2f;
		ssaoCB.OcclusionFadeEnd = 1.0f;
		ssaoCB.SurfaceEpsilon = 0.25f;

		m_constantBuffers[param.FrameResourceIndex]->CopyData(0, ssaoCB);
	}

	void ScreenSpaceAO::UpdateBlurConstants()
	{
		BlurConstants blurCB;
		memcpy(blurCB.BlurWeights, m_blurWeights, sizeof(m_blurWeights));

		m_blurConstantBuffer->CopyData(0, blurCB);
	}

	void ScreenSpaceAO::ClearSSAOMap(ID3D12GraphicsCommandList* pCommandList)
	{
		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		pCommandList->ClearRenderTargetView(m_ssaomapCpuRtv, clearColor, 0, nullptr);

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void ScreenSpaceAO::PassSSAO(RenderParam& param)
	{
		auto pCommandList = param.CommandList;

		pCommandList->RSSetViewports(1, &m_ssaoViewport);
		pCommandList->RSSetScissorRects(1, &m_ssaoScissorRect);

		// Transition the normal map and ambient occlusion map to render target state
		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		pCommandList->OMSetRenderTargets(1, &m_ssaomapCpuRtv, true, nullptr);

		pCommandList->SetGraphicsRootSignature(m_ssaoRootSignature.Get());
		pCommandList->SetGraphicsRootConstantBufferView(0, m_constantBuffers[param.FrameResourceIndex]->Resource()->GetGPUVirtualAddress());
		pCommandList->SetGraphicsRootDescriptorTable(1, param.Renderer->GetGBufferSrv(1));
		pCommandList->SetGraphicsRootDescriptorTable(2, param.Renderer->GetDepthBufferSrv());

		pCommandList->SetPipelineState(m_ssaoPSO.Get());

		// Draw a quad, which will cover the entire screen (by the vertex shader)
		pCommandList->IASetVertexBuffers(0, 0, nullptr);
		pCommandList->IASetIndexBuffer(nullptr);
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCommandList->DrawInstanced(6, 1, 0, 0);

		// Transition the normal map and ambient occlusion map to generic read state
		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void ScreenSpaceAO::PassBlur(RenderParam& param)
	{
		auto pCommandList = param.CommandList;

		pCommandList->SetComputeRootSignature(m_blurRootSignature.Get());
		pCommandList->SetPipelineState(m_blurPSO.Get());

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_blurMap.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		pCommandList->SetComputeRootConstantBufferView(0, m_constantBuffers[param.FrameResourceIndex]->Resource()->GetGPUVirtualAddress());
		pCommandList->SetComputeRoot32BitConstant(1, 0, 0);
		pCommandList->SetComputeRootConstantBufferView(2, m_blurConstantBuffer->Resource()->GetGPUVirtualAddress());
		pCommandList->SetComputeRootDescriptorTable(3, m_ssaomapGpuSrv);
		pCommandList->SetComputeRootDescriptorTable(4, param.Renderer->GetGBufferSrv(1));
		pCommandList->SetComputeRootDescriptorTable(5, param.Renderer->GetDepthBufferSrv());
		pCommandList->SetComputeRootDescriptorTable(6, m_blurMapGpuUav);

		constexpr UINT ThreadGroupSize = 32;

		pCommandList->Dispatch((m_width + ThreadGroupSize - 1) / ThreadGroupSize, m_height, 1);

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_blurMap.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_GENERIC_READ));

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		pCommandList->SetComputeRootConstantBufferView(0, m_constantBuffers[param.FrameResourceIndex]->Resource()->GetGPUVirtualAddress());
		pCommandList->SetComputeRoot32BitConstant(1, 1, 0);
		pCommandList->SetComputeRootConstantBufferView(2, m_blurConstantBuffer->Resource()->GetGPUVirtualAddress());
		pCommandList->SetComputeRootDescriptorTable(3, m_blurMapGpuSrv);
		pCommandList->SetComputeRootDescriptorTable(4, param.Renderer->GetGBufferSrv(1));
		pCommandList->SetComputeRootDescriptorTable(5, param.Renderer->GetDepthBufferSrv());
		pCommandList->SetComputeRootDescriptorTable(6, m_ssaoMapGpuUav);

		pCommandList->Dispatch((m_height + ThreadGroupSize - 1) / ThreadGroupSize, m_width, 1);

		pCommandList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_ssaomap.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void ScreenSpaceAO::OnResize(UINT newWidth, UINT newHeight, ID3D12Device* device)
	{
		m_width = newWidth;
		m_height = newHeight;

		// Resize the normal map and ambient occlusion map
		m_ssaoViewport.TopLeftX = 0.0f;
		m_ssaoViewport.TopLeftY = 0.0f;
		m_ssaoViewport.Width = m_width * m_ssaoMapScale;
		m_ssaoViewport.Height = m_height * m_ssaoMapScale;
		m_ssaoViewport.MinDepth = 0.0f;
		m_ssaoViewport.MaxDepth = 1.0f;

		m_ssaoScissorRect = { 0, 0, (int)m_ssaoViewport.Width, (int)m_ssaoViewport.Height };

		// Create the normal map and ambient occlusion map
		BuildResources();
	}

	void ScreenSpaceAO::BuildResources()
	{
		// Free the old resources if they exist
		m_ssaomap.Reset();
		m_blurMap.Reset();

		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = AO_FORMAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		// Ambient map and blur map
		float aoClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		CD3DX12_CLEAR_VALUE clearValue(AO_FORMAT, aoClearColor);

		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_blurMap.GetAddressOf())));

		texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		// SSAO map
		texDesc.Width = static_cast<UINT>(m_width * m_ssaoMapScale);
		texDesc.Height = static_cast<UINT>(m_height * m_ssaoMapScale);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(m_ssaomap.GetAddressOf())));
	}

	void ScreenSpaceAO::BuildDescriptors(DescriptorParam& descriptorParam, ID3D12Resource* depthStencilBuffer)
	{
		m_ssaomapCpuSrv = descriptorParam.SrvCpuHandle;
		m_blurMapCpuSrv = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_depthMapCpuSrv = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_ssaomapGpuSrv = descriptorParam.SrvGpuHandle;
		m_blurMapGpuSrv = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_depthMapGpuSrv = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_ssaoMapCpuUav = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_blurMapCpuUav = descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_ssaoMapGpuUav = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		m_blurMapGpuUav = descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		m_ssaomapCpuRtv = descriptorParam.RtvCpuHandle.Offset(1, descriptorParam.RtvDescriptorSize);

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.RtvCpuHandle.Offset(1, descriptorParam.RtvDescriptorSize);

		RebuildDescriptors(depthStencilBuffer);
	}

	void ScreenSpaceAO::RebuildDescriptors(ID3D12Resource* depthStencilBuffer)
	{
		// Create the normal map and ambient occlusion map shader resource views
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(depthStencilBuffer, &srvDesc, m_depthMapCpuSrv);

		srvDesc.Format = AO_FORMAT;
		m_device->CreateShaderResourceView(m_ssaomap.Get(), &srvDesc, m_ssaomapCpuSrv);
		m_device->CreateShaderResourceView(m_blurMap.Get(), &srvDesc, m_blurMapCpuSrv);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = AO_FORMAT;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_device->CreateUnorderedAccessView(m_ssaomap.Get(), nullptr, &uavDesc, m_ssaoMapCpuUav);
		m_device->CreateUnorderedAccessView(m_blurMap.Get(), nullptr, &uavDesc, m_blurMapCpuUav);

		// Create the render target view
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = AO_FORMAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		m_device->CreateRenderTargetView(m_ssaomap.Get(), &rtvDesc, m_ssaomapCpuRtv);
	}

	void ScreenSpaceAO::BuildRootSignature(ID3D12Device* pDevice)
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
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
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

		std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
		{
			pointClamp, linearClamp, depthMapSam, linearWrap
		};

		{
			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable2;
			texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[3];

			// Perfomance TIP: Order from most frequent to least frequent.
			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
				(UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
			ComPtr<ID3DBlob> serializedRootSig = nullptr;
			ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			ThrowIfFailed(hr);

			ThrowIfFailed(pDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(m_ssaoRootSignature.GetAddressOf())));
		}

		{
			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable2;
			texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			CD3DX12_DESCRIPTOR_RANGE texTable3;
			texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

			CD3DX12_DESCRIPTOR_RANGE texTable4;
			texTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			CD3DX12_ROOT_PARAMETER slotRootParameter[7];

			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsConstants(1, 1);
			slotRootParameter[2].InitAsConstantBufferView(2);
			slotRootParameter[3].InitAsDescriptorTable(1, &texTable1);
			slotRootParameter[4].InitAsDescriptorTable(1, &texTable2);
			slotRootParameter[5].InitAsDescriptorTable(1, &texTable3);
			slotRootParameter[6].InitAsDescriptorTable(1, &texTable4);

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
				(UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
			ComPtr<ID3DBlob> serializedRootSig = nullptr;
			ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			ThrowIfFailed(hr);

			ThrowIfFailed(pDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(m_blurRootSignature.GetAddressOf())));
		}
	}

	void ScreenSpaceAO::BuildPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature)
	{
		{
			// Build the SSAO PSO
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_screenspace_ao.cso");

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

			// There's no vertex input in this case
			psoDesc.InputLayout.pInputElementDescs = nullptr;
			psoDesc.InputLayout.NumElements = 0;
			psoDesc.pRootSignature = m_ssaoRootSignature.Get();
			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(vsByteCode.data()),
				vsByteCode.size()
			};
			psoDesc.PS =
			{
				reinterpret_cast<BYTE*>(psByteCode.data()),
				psByteCode.size()
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
			psoDesc.RTVFormats[0] = AO_FORMAT;
			psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

			ThrowIfFailed(pDevice->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_ssaoPSO.GetAddressOf())
			));
			m_ssaoPSO->SetName(L"ScreenSpaceAO::PassSSAO");
		}

		{
			// Build the blur PSO
			auto csByteCode = DX::ReadData(L"compiled_shaders\\cs_screenspace_ao_blur.cso");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			// There's no vertex input in this case
			psoDesc.pRootSignature = m_blurRootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode.data()),
				csByteCode.size()
			};

			ThrowIfFailed(pDevice->CreateComputePipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_blurPSO.GetAddressOf())
			));
			m_blurPSO->SetName(L"ScreenSpaceAO::PassBlur");
		}
	}

	void ScreenSpaceAO::BuildOffsetVectors()
	{
		std::default_random_engine dre;
		std::uniform_real_distribution<float> urdx(-1.0f, 1.0f);

		for (int i = 0; i < KERNEL_SIZE; ++i)
		{
			float scale = float(i) / KERNEL_SIZE;
			scale = std::lerp(0.1f, 1.0f, scale * scale);
			XMVECTOR v = XMVectorSet(urdx(dre), urdx(dre), urdx(dre) * 0.5f + 0.5f, 0.0f);
			v = XMVector3Normalize(v) * scale;
			XMStoreFloat4(&m_offsets[i], v);
		}
	}

	void ScreenSpaceAO::BuildBlurWeights()
	{
		// Blur weights with a Gaussian distribution
		const float sigma = BLUR_SMAPLE / 2.0f;

		for (int x = 0; x < BLUR_SMAPLE; ++x)
			m_blurWeights[x] = std::expf(-0.5f * (std::powf(x / sigma, 2.0f))) / (sigma * std::sqrtf(2.0f * XM_PI));
	}
}