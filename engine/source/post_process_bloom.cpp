#include "pch.h"
#include "post_process_bloom.h"
#include "deferred_renderer.h"

namespace udsdx
{
	PostProcessBloom::PostProcessBloom(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
	{
		m_device = device;
		BuildRootSignature();
	}

	void PostProcessBloom::Pass(RenderParam& param)
	{
		auto pCommandList = param.CommandList;

		ID3D12Resource* source = param.Renderer->GetRenderTargetResource();

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			source,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

		pCommandList->IASetVertexBuffers(0, 0, nullptr);
		pCommandList->IASetIndexBuffer(nullptr);
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCommandList->SetPipelineState(m_psoPrepass.Get());

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_bloomBuffers[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

		pCommandList->OMSetRenderTargets(1, &m_bloomCpuRtv[0], true, nullptr);

		pCommandList->RSSetViewports(1, &m_viewports[0]);
		pCommandList->RSSetScissorRects(1, &m_scissorRects[0]);

		pCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_bloomThreshold, 0);
		pCommandList->SetGraphicsRootDescriptorTable(1, param.Renderer->GetRenderTargetSrv());
		pCommandList->DrawInstanced(6, 1, 0, 0);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_bloomBuffers[0].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		pCommandList->SetPipelineState(m_psoDownsample.Get());

		// Index indicates the source mip level.
		for (UINT index = 0; index < NumMipChains - 1; ++index)
		{
			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_bloomBuffers[index + 1].Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

			pCommandList->OMSetRenderTargets(1, &m_bloomCpuRtv[index + 1], true, nullptr);

			pCommandList->RSSetViewports(1, &m_viewports[index + 1]);
			pCommandList->RSSetScissorRects(1, &m_scissorRects[index + 1]);

			pCommandList->SetGraphicsRootDescriptorTable(1, m_bloomGpuSrv[index]);
			pCommandList->DrawInstanced(6, 1, 0, 0);

			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_bloomBuffers[index + 1].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		pCommandList->SetPipelineState(m_psoUpsample.Get());
		pCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_upsampleScale, 0);

		// Index indicates the source mip level.
		for (UINT index = NumMipChains - 1; index > 0; --index)
		{
			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_bloomBuffers[index - 1].Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

			pCommandList->OMSetRenderTargets(1, &m_bloomCpuRtv[index - 1], true, nullptr);

			pCommandList->RSSetViewports(1, &m_viewports[index - 1]);
			pCommandList->RSSetScissorRects(1, &m_scissorRects[index - 1]);

			pCommandList->SetGraphicsRootDescriptorTable(1, m_bloomGpuSrv[index]);
			pCommandList->DrawInstanced(6, 1, 0, 0);

			pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_bloomBuffers[index - 1].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		pCommandList->OMSetRenderTargets(1, &param.RenderTargetView, true, nullptr);

		pCommandList->SetPipelineState(m_psoSynthesize.Get());
		pCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_exposure, 0);
		pCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_bloomStrength, 1);
		pCommandList->SetGraphicsRootDescriptorTable(1, param.Renderer->GetRenderTargetSrv());
		pCommandList->SetGraphicsRootDescriptorTable(2, m_bloomGpuSrv[0]);
		pCommandList->DrawInstanced(6, 1, 0, 0);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			source,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));
	}

	void PostProcessBloom::OnResize(UINT newWidth, UINT newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;

		for (UINT index = 0; index < NumMipChains; ++index)
		{
			m_viewports[index] = { 0.0f, 0.0f, static_cast<float>(m_width >> index), static_cast<float>(m_height >> index), 0.0f, 1.0f };
			m_scissorRects[index] = { 0, 0, static_cast<LONG>(m_width >> index), static_cast<LONG>(m_height >> index) };
		}

		BuildResources();
	}

	void PostProcessBloom::BuildResources()
	{
		for (UINT index = 0; index < NumMipChains; ++index)
		{
			m_bloomBuffers[index].Reset();

			D3D12_RESOURCE_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));

			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width =	m_width >> index;
			texDesc.Height = m_height >> index;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.Format = BloomFormat;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = BloomFormat;
			memcpy(clearValue.Color, BloomClearValue, sizeof(BloomClearValue));

			ThrowIfFailed(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&clearValue,
				IID_PPV_ARGS(&m_bloomBuffers[index])));
		}
	}

	void PostProcessBloom::BuildDescriptors(DescriptorParam& descriptorParam)
	{
		for (UINT index = 0; index < NumMipChains; ++index)
		{
			m_bloomCpuSrv[index] = descriptorParam.SrvCpuHandle;
			m_bloomGpuSrv[index] = descriptorParam.SrvGpuHandle;
			m_bloomCpuRtv[index] = descriptorParam.RtvCpuHandle;

			descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
			descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
			descriptorParam.RtvCpuHandle.Offset(1, descriptorParam.RtvDescriptorSize);
		}

		RebuildDescriptors();
	}

	void PostProcessBloom::RebuildDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = BloomFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = BloomFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		for (UINT index = 0; index < NumMipChains; ++index)
		{
			m_device->CreateShaderResourceView(m_bloomBuffers[index].Get(), &srvDesc, m_bloomCpuSrv[index]);
			m_device->CreateRenderTargetView(m_bloomBuffers[index].Get(), &rtvDesc, m_bloomCpuRtv[index]);
		}
	}

	void PostProcessBloom::BuildRootSignature()
	{
		{
			CD3DX12_ROOT_PARAMETER slotRootParameter[3]{};

			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE texTable2;
			texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			slotRootParameter[0].InitAsConstants(2, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

			CD3DX12_STATIC_SAMPLER_DESC samplerDesc[] = {
				CD3DX12_STATIC_SAMPLER_DESC(
				0,
				D3D12_FILTER_MIN_MAG_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP),

				CD3DX12_STATIC_SAMPLER_DESC(
				1,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP)
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
	}

	void PostProcessBloom::BuildPipelineState()
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
		psoDesc.RTVFormats[0] = DestinationFormat;
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_bloom_synthesize.cso");

			psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
			psoDesc.PS = { psByteCode.data(), psByteCode.size() };

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_psoSynthesize.GetAddressOf())));
			m_psoSynthesize->SetName(L"PostProcessBloom::PassSynthesize");
		}

		psoDesc.RTVFormats[0] = BloomFormat;

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_bloom_prepass.cso");

			psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
			psoDesc.PS = { psByteCode.data(), psByteCode.size() };

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_psoPrepass.GetAddressOf())));
			m_psoPrepass->SetName(L"PostProcessBloom::PassDownSample");
		}

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_bloom_downsample.cso");

			psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
			psoDesc.PS = { psByteCode.data(), psByteCode.size() };

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_psoDownsample.GetAddressOf())));
			m_psoDownsample->SetName(L"PostProcessBloom::PassDownSample");
		}

		psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_drawscreen.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_bloom_upsample.cso");

			psoDesc.VS = { vsByteCode.data(), vsByteCode.size() };
			psoDesc.PS = { psByteCode.data(), psByteCode.size() };

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_psoUpsample.GetAddressOf())));
			m_psoUpsample->SetName(L"PostProcessBloom::PassUpSample");
		}
	}
}