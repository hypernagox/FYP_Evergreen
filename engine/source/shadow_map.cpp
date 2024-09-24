#include "pch.h"
#include "shadow_map.h"
#include "frame_resource.h"
#include "camera.h"
#include "light_directional.h"
#include "scene_object.h"
#include "transform.h"
#include "scene.h"

namespace udsdx
{
	ShadowMap::ShadowMap(UINT mapWidth, UINT mapHeight, ID3D12Device* device)
	{
		m_mapWidth = mapWidth;
		m_mapHeight = mapHeight;

		m_viewport = { 0.0f, 0.0f, (float)mapWidth, (float)mapHeight, 0.0f, 1.0f };
		m_scissorRect = { 0, 0, (int)mapWidth, (int)mapHeight };

		OnResize(mapWidth, mapHeight, device);
		for (auto& buffer : m_constantBuffers)
		{
			buffer = std::make_unique<UploadBuffer<ShadowConstants>>(device, 1, true);
		}
	}

	ShadowMap::~ShadowMap()
	{

	}

	void ShadowMap::OnResize(UINT newWidth, UINT newHeight, ID3D12Device* device)
	{
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = newWidth;
		texDesc.Height = newHeight;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = m_shadowMapFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(&m_shadowMap)));
	}

	void ShadowMap::BuildDescriptors(DescriptorParam& descriptorParam, ID3D12Device* device)
	{
		m_srvCpu = descriptorParam.SrvCpuHandle;
		m_dsvCpu = descriptorParam.DsvCpuHandle;
		m_srvGpu = descriptorParam.SrvGpuHandle;

		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.DsvCpuHandle.Offset(1, descriptorParam.DsvDescriptorSize);
		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		// Create SRV to resource so we can sample the shadow map in a shader program.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;
		device->CreateShaderResourceView(m_shadowMap.Get(), &srvDesc, m_srvCpu);

		// Create DSV to resource so we can render to the shadow map.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		device->CreateDepthStencilView(m_shadowMap.Get(), &dsvDesc, m_dsvCpu);
	}

	void ShadowMap::Pass(RenderParam& param, Scene* target, Camera* camera, LightDirectional* light)
	{
		auto pCommandList = param.CommandList;

		pCommandList->SetGraphicsRootSignature(param.RootSignature);

		pCommandList->RSSetViewports(1, &m_viewport);
		pCommandList->RSSetScissorRects(1, &m_scissorRect);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		pCommandList->ClearDepthStencilView(m_dsvCpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); 
		pCommandList->OMSetRenderTargets(0, nullptr, false, &m_dsvCpu);

		// Bind the current frame's constant buffer to the pipeline.
		pCommandList->SetGraphicsRootConstantBufferView(RootParam::PerFrameCBV, param.ConstantBufferView);
		pCommandList->SetGraphicsRootDescriptorTable(RootParam::ShadowMapSRV, m_srvGpu);

		ShadowConstants shadowConstants;
		Vector3 lightDirection = light->GetLightDirection();
		Vector3 cameraPos = Vector3::Transform(Vector3::Zero, camera->GetSceneObject()->GetTransform()->GetWorldSRTMatrix());
		XMMATRIX lightView = XMMatrixLookAtLH(cameraPos, XMLoadFloat3(&(cameraPos + lightDirection)), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		for (int i = 0; i < 4; ++i)
		{
			float f = m_shadowRanges[i];
			XMMATRIX lightProj = XMMatrixOrthographicLH(f, f, -f * 10.0f, f * 10.0f);
			XMMATRIX lightClip = XMMatrixScaling(0.5f, 0.5f, 1.0f) * XMMatrixTranslation(static_cast<float>(i % 2) - 0.5f, static_cast<float>(i / 2) - 0.5f, 0.0f);
			XMMATRIX lightViewProj = lightView * lightProj;
			XMStoreFloat4x4(&shadowConstants.LightViewProj[i], XMMatrixTranspose(lightViewProj));
			XMStoreFloat4x4(&shadowConstants.LightViewProjClip[i], XMMatrixTranspose(lightViewProj * lightClip));

			shadowConstants.ShadowBias[i] = f;
			shadowConstants.ShadowDistance[i] = f * 0.5f;
		}
		shadowConstants.LightDirection = lightDirection;

		m_constantBuffers[param.FrameResourceIndex]->CopyData(0, shadowConstants);

		param.CommandList->SetGraphicsRootConstantBufferView(3, m_constantBuffers[param.FrameResourceIndex]->Resource()->GetGPUVirtualAddress());
		param.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		param.UseFrustumCulling = false;
		target->RenderShadowSceneObjects(param, 4);
		param.UseFrustumCulling = true;

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	D3D12_GPU_VIRTUAL_ADDRESS ShadowMap::GetConstantBuffer(int frameResourceIndex) const
	{
		return m_constantBuffers[frameResourceIndex]->Resource()->GetGPUVirtualAddress();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE ShadowMap::GetSrvGpu() const
	{
		return m_srvGpu;
	}

	void ShadowMap::SetShadowRange(UINT index, float value)
	{
		m_shadowRanges[index] = value;
	}
}