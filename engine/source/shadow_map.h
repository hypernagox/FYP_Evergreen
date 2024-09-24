#pragma once

#include "pch.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;
	class Camera;
	class LightDirectional;

	class ShadowMap
	{
	public:
		ShadowMap(UINT mapWidth, UINT mapHeight, ID3D12Device* device);
		~ShadowMap();

	public:
		void OnResize(UINT newWidth, UINT newHeight, ID3D12Device* device);
		void BuildDescriptors(DescriptorParam& descriptorParam, ID3D12Device* device);

		void Pass(RenderParam& param, Scene* target, Camera* camera, LightDirectional* light);

	public:
		D3D12_GPU_VIRTUAL_ADDRESS GetConstantBuffer(int frameResourceIndex) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpu() const;

		void SetShadowRange(UINT index, float value);

	protected:
		DXGI_FORMAT m_shadowMapFormat = DXGI_FORMAT_R24G8_TYPELESS;

		UINT m_mapWidth;
		UINT m_mapHeight;

		float m_shadowRanges[4] = { 10.0f, 30.0f, 120.0f, 1280.0f };

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

		D3D12_CPU_DESCRIPTOR_HANDLE m_dsvCpu;
		D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpu;

		ComPtr<ID3D12Resource> m_shadowMap;

		std::array<std::unique_ptr<UploadBuffer<ShadowConstants>>, FrameResourceCount> m_constantBuffers;
	};
}