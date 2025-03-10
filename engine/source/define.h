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
	class MotionBlur;
	class BoundingCamera;

	struct RenderParam
	{
		ID3D12Device* Device;
		ID3D12GraphicsCommandList* CommandList;
		ID3D12RootSignature* RootSignature;
		ID3D12DescriptorHeap* SRVDescriptorHeap;

		DeferredRenderer* Renderer;

		float AspectRatio;
		int FrameResourceIndex;
		int RenderStageIndex;
		const Time& Time;

		const D3D12_VIEWPORT& Viewport;
		const D3D12_RECT& ScissorRect;

		BoundingCamera* ViewFrustumWorld;
		bool UseFrustumCulling;

		const D3D12_GPU_VIRTUAL_ADDRESS& ConstantBufferView;
		const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView;
		const D3D12_CPU_DESCRIPTOR_HANDLE& RenderTargetView;
		ID3D12Resource* RenderTargetResource;

		ShadowMap* RenderShadowMap;
		ScreenSpaceAO* RenderScreenSpaceAO;
		MotionBlur* RenderMotionBlur;

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
		PerObjectCBV,
		PerCameraCBV,
		BonesCBV,
		PrevBonesCBV,
		PerShadowCBV,
		PerFrameCBV,
		MainTexSRV,
		NormalSRV,
		ShadowMapSRV,
		SSAOMapSRV
	};

	enum RenderGroup : UINT
	{
		Forward = 0,
		Deferred = 1
	};

	static constexpr int FrameResourceCount = 2;
	static constexpr int SwapChainBufferCount = 2;
}