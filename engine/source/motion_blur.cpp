#include "pch.h"
#include "motion_blur.h"
#include "deferred_renderer.h"
#include "camera.h"
#include "shader_compile.h"

namespace udsdx
{
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
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

		pCommandList->CopyResource(m_sourceBuffer.Get(), param.RenderTargetResource);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(param.RenderTargetResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sourceBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Set the render target
		pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());
		pCommandList->OMSetRenderTargets(1, &param.RenderTargetView, true, nullptr);

		pCommandList->RSSetViewports(1, &param.Viewport);
		pCommandList->RSSetScissorRects(1, &param.ScissorRect);

		pCommandList->IASetVertexBuffers(0, 0, nullptr);
		pCommandList->IASetIndexBuffer(nullptr);
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCommandList->SetPipelineState(m_pso.Get());
		pCommandList->SetGraphicsRootConstantBufferView(0, cbvGpu);
		pCommandList->SetGraphicsRootDescriptorTable(1, m_sourceGpuSrv);
		pCommandList->SetGraphicsRootDescriptorTable(2, param.Renderer->GetGBufferSrv(2));
		pCommandList->SetGraphicsRootDescriptorTable(3, param.Renderer->GetDepthBufferSrv());
		pCommandList->SetGraphicsRootDescriptorTable(4, m_neighborMaxGpuSrv);
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
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(m_sourceBuffer.GetAddressOf())));
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

			CD3DX12_ROOT_PARAMETER slotRootParameter[5]{};

			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable1);
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable2);
			slotRootParameter[3].InitAsDescriptorTable(1, &texTable3);
			slotRootParameter[4].InitAsDescriptorTable(1, &texTable4);

			CD3DX12_STATIC_SAMPLER_DESC samplerDesc[] = {
				CD3DX12_STATIC_SAMPLER_DESC(
				0,
				D3D12_FILTER_MIN_MAG_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			};

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, _countof(samplerDesc), samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
		{
			auto csByteCode = DX::ReadData(L"compiled_shaders\\cs_motion_blur_tilemax.cso");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			psoDesc.pRootSignature = m_computeRootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode.data()),
				csByteCode.size()
			};

			ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_tileMaxPso.GetAddressOf())));
			m_tileMaxPso->SetName(L"MotionBlur::TileMaxPass");
		}

		{
			auto csByteCode = DX::ReadData(L"compiled_shaders\\cs_motion_blur_neighbormax.cso");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

			psoDesc.pRootSignature = m_computeRootSignature.Get();
			psoDesc.CS =
			{
				reinterpret_cast<BYTE*>(csByteCode.data()),
				csByteCode.size()
			};

			ThrowIfFailed(m_device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_neighborMaxPso.GetAddressOf())));
			m_neighborMaxPso->SetName(L"MotionBlur::NeighborMaxPass");
		}

		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

			psoDesc.InputLayout.pInputElementDescs = nullptr;
			psoDesc.InputLayout.NumElements = 0;
			psoDesc.pRootSignature = m_rootSignature.Get();
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

			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_motion_blur_pass.cso");

			psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
			psoDesc.PS = { psByteCode.data(), psByteCode.size() };

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pso.GetAddressOf())));
			m_pso->SetName(L"MotionBlur::Pass");
		}
	}
}