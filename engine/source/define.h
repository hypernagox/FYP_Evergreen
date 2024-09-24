#pragma once

#include "pch.h"

namespace udsdx
{
	using Vector2 = DirectX::SimpleMath::Vector2;
	using Vector2Int = XMINT2;
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector3Int = XMINT3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Vector4Int = XMINT4;
	using Color = DirectX::SimpleMath::Color;
	using Plane = DirectX::SimpleMath::Plane;
	using Quaternion = DirectX::SimpleMath::Quaternion;
	using Matrix4x4 = DirectX::SimpleMath::Matrix;

	struct Time
	{
		float deltaTime;
		float totalTime;
	};

	class ShadowMap;
	class ScreenSpaceAO;
	class DeferredRenderer;

	struct RenderParam
	{
		ID3D12Device* Device;
		ID3D12GraphicsCommandList* CommandList;
		ID3D12RootSignature* RootSignature;
		ID3D12DescriptorHeap* SRVDescriptorHeap;

		DeferredRenderer* Renderer;

		float AspectRatio;
		int FrameResourceIndex;

		const D3D12_VIEWPORT& Viewport;
		const D3D12_RECT& ScissorRect;

		BoundingFrustum ViewFrustumWorld;
		bool UseFrustumCulling;

		const D3D12_GPU_VIRTUAL_ADDRESS& ConstantBufferView;
		const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView;
		const D3D12_CPU_DESCRIPTOR_HANDLE& RenderTargetView;

		ShadowMap* RenderShadowMap;
		ScreenSpaceAO* RenderScreenSpaceAO;

		TracyD3D12Ctx* TracyQueueContext;
	};

	struct DescriptorParam
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE CbvCpuHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE SrvCpuHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE DsvCpuHandle;

		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvGpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGpuHandle;

		UINT CbvSrvUavDescriptorSize;
		UINT RtvDescriptorSize;
		UINT DsvDescriptorSize;
	};

	enum RootParam : UINT
	{
		PerObjectCBV = 0,
		PerCameraCBV = 1,
		BonesCBV = 2,
		PerShadowCBV = 3,
		PerFrameCBV = 4,
		MainTexSRV = 5,
		NormalSRV = 6,
		ShadowMapSRV = 7,
		SSAOMapSRV = 8
	};

	static constexpr int FrameResourceCount = 2;
	static constexpr int SwapChainBufferCount = 2;
}