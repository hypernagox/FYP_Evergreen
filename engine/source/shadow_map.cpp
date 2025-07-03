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
		OnResize(mapWidth, mapHeight, device);

		for (auto& buffer : m_constantBuffers)
		{
			buffer = std::make_unique<UploadBuffer<ShadowConstants>>(device, 1, true);
		}
		for (auto& bufferSet : m_lightCameraBuffers)
		{
			for (auto& buffer : bufferSet)
			{
				buffer = std::make_unique<UploadBuffer<CameraConstants>>(device, 1, true);
			}
		}
	}

	ShadowMap::~ShadowMap()
	{

	}

	void ShadowMap::OnResize(UINT newWidth, UINT newHeight, ID3D12Device* device)
	{
		m_mapWidth = newWidth;
		m_mapHeight = newHeight;

		m_viewport = { 0.0f, 0.0f, (float)newWidth, (float)newHeight, 0.0f, 1.0f };
		m_scissorRect = { 0, 0, (int)newWidth, (int)newHeight };

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

		RebuildDescriptors(device);
	}

	void ShadowMap::RebuildDescriptors(ID3D12Device* device)
	{
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

		pCommandList->RSSetViewports(1, &m_viewport);
		pCommandList->RSSetScissorRects(1, &m_scissorRect);

		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		pCommandList->ClearDepthStencilView(m_dsvCpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); 
		pCommandList->OMSetRenderTargets(0, nullptr, false, &m_dsvCpu);

		pCommandList->SetGraphicsRootConstantBufferView(RootParam::PerShadowCBV, GetConstantBuffer(param.FrameResourceIndex));
		pCommandList->SetGraphicsRootConstantBufferView(RootParam::PerFrameCBV, param.ConstantBufferView);

		ShadowConstants shadowConstants;
		Vector3 lightDirection = light->GetLightDirection();
		std::array<std::unique_ptr<BoundingCamera>, 4> cameraBounds;
		Vector3 cameraPos = camera->GetTransform()->GetWorldPosition();
		Vector3 cameraLook = Vector3::TransformNormal(Vector3::Backward, Matrix4x4::CreateFromQuaternion(camera->GetTransform()->GetWorldRotation()));

		for (int i = 0; i < 4; ++i)
		{
			float f = m_shadowRanges[i];
			Vector3 lightPos = cameraPos + cameraLook * f * 0.5f;
			XMMATRIX lightView = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&(lightPos + lightDirection)), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			XMVECTOR texelUnit = XMVectorSet(f * 2.0f / static_cast<float>(m_mapWidth), f * 2.0f / static_cast<float>(m_mapHeight), 1.0f, 1.0f);
			lightView.r[3] = XMVectorSelect(lightView.r[3], XMVectorFloor(lightView.r[3] / texelUnit) * texelUnit, XMVectorSelectControl(true, true, false, false));
			XMMATRIX lightProj = XMMatrixOrthographicLH(f, f, -f * 10.0f, f * 10.0f);
			XMMATRIX lightClip = XMMatrixScaling(0.5f, 0.5f, 1.0f) * XMMatrixTranslation(static_cast<float>(i % 2) - 0.5f, static_cast<float>(i / 2) - 0.5f, 0.0f);
			XMMATRIX lightViewProj = lightView * lightProj;
			XMStoreFloat4x4(&shadowConstants.LightViewProj[i], XMMatrixTranspose(lightViewProj));
			XMStoreFloat4x4(&shadowConstants.LightViewProjClip[i], XMMatrixTranspose(lightViewProj * lightClip));
			XMStoreFloat4(&shadowConstants.LightPosition[i], XMVectorSet(lightPos.x, lightPos.y, lightPos.z, 0.0f));
			shadowConstants.ShadowDistance[i] = f * 0.5f;

			CameraConstants cameraConstants{};
			XMStoreFloat4x4(&cameraConstants.View, XMMatrixTranspose(lightView));
			XMStoreFloat4x4(&cameraConstants.Proj, XMMatrixTranspose(lightProj));
			XMStoreFloat4x4(&cameraConstants.ViewProj, XMMatrixTranspose(lightViewProj));
			m_lightCameraBuffers[param.FrameResourceIndex][i]->CopyData(0, cameraConstants);

			Matrix4x4 mView;
			XMStoreFloat4x4(&mView, lightView);	
			cameraBounds[i] = std::make_unique<BoundingCameraOrthographic>(mView, f, f, -f * 10.0f, f * 10.0f);
		}
		shadowConstants.LightDirection = lightDirection;

		m_constantBuffers[param.FrameResourceIndex]->CopyData(0, shadowConstants);

		param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerShadowCBV, m_constantBuffers[param.FrameResourceIndex]->Resource()->GetGPUVirtualAddress());

		if (param.RenderOptions->DrawShadowMap)
		{
			param.UseFrustumCulling = false;

			D3D12_VIEWPORT tempViewport{};
			D3D12_RECT tempScissorRect{};

			int halfWidth = m_mapWidth / 2;
			int halfHeight = m_mapHeight / 2;

			tempViewport = { 0.0f, (float)halfHeight, (float)halfWidth, (float)halfHeight, 0.0f, 1.0f };
			tempScissorRect = { 0, halfHeight, halfWidth, halfHeight * 2 };

			param.ViewFrustumWorld = cameraBounds[0].get();
			param.CommandList->RSSetViewports(1, &tempViewport);
			param.CommandList->RSSetScissorRects(1, &tempScissorRect);
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, m_lightCameraBuffers[param.FrameResourceIndex][0]->Resource()->GetGPUVirtualAddress());
			target->RenderShadowSceneObjects(param, 1);

			tempViewport = { (float)halfWidth, (float)halfHeight, (float)halfWidth, (float)halfHeight, 0.0f, 1.0f };
			tempScissorRect = { halfWidth, halfHeight, halfWidth * 2, halfHeight * 2 };

			param.ViewFrustumWorld = cameraBounds[1].get();
			param.CommandList->RSSetViewports(1, &tempViewport);
			param.CommandList->RSSetScissorRects(1, &tempScissorRect);
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, m_lightCameraBuffers[param.FrameResourceIndex][1]->Resource()->GetGPUVirtualAddress());
			target->RenderShadowSceneObjects(param, 1);

			tempViewport = { 0.0f, 0.0f, (float)halfWidth, (float)halfHeight, 0.0f, 1.0f };
			tempScissorRect = { 0, 0, halfWidth, halfHeight };

			param.ViewFrustumWorld = cameraBounds[2].get();
			param.CommandList->RSSetViewports(1, &tempViewport);
			param.CommandList->RSSetScissorRects(1, &tempScissorRect);
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, m_lightCameraBuffers[param.FrameResourceIndex][2]->Resource()->GetGPUVirtualAddress());
			target->RenderShadowSceneObjects(param, 1);

			tempViewport = { (float)halfWidth, 0.0f, (float)halfWidth, (float)halfHeight, 0.0f, 1.0f };
			tempScissorRect = { halfWidth, 0, halfWidth * 2, halfHeight };

			param.ViewFrustumWorld = cameraBounds[3].get();
			param.CommandList->RSSetViewports(1, &tempViewport);
			param.CommandList->RSSetScissorRects(1, &tempScissorRect);
			param.CommandList->SetGraphicsRootConstantBufferView(RootParam::PerCameraCBV, m_lightCameraBuffers[param.FrameResourceIndex][3]->Resource()->GetGPUVirtualAddress());
			target->RenderShadowSceneObjects(param, 1);

			param.UseFrustumCulling = true;
		}

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