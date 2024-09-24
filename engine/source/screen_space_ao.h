#pragma once

#include "pch.h"

namespace udsdx
{
	class Scene;
	class Camera;

	class ScreenSpaceAO
	{
	public:
		static constexpr int KERNEL_SIZE = 32;
		static constexpr int BLUR_SMAPLE = 32; // should be a multiple of 4

		static constexpr DXGI_FORMAT AO_FORMAT = DXGI_FORMAT_R8_UNORM;
		static constexpr DXGI_FORMAT NORMAL_FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;

		struct SSAOConstants
		{
			Matrix4x4 Proj = Matrix4x4::Identity;
			Matrix4x4 InvProj = Matrix4x4::Identity;
			Matrix4x4 ProjTex = Matrix4x4::Identity;
			Vector4 OffsetVectors[ScreenSpaceAO::KERNEL_SIZE];
			float OcclusionRadius = 0.5f;
			float OcclusionFadeStart = 0.2f;
			float OcclusionFadeEnd = 1.0f;
			float SurfaceEpsilon = 0.05f;
		};

		struct BlurConstants
		{
			float BlurWeights[ScreenSpaceAO::BLUR_SMAPLE];
		};

	public:
		ScreenSpaceAO(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, float ssaoMapScale = 1.0f);
		ScreenSpaceAO(const ScreenSpaceAO& rhs) = delete;
		ScreenSpaceAO& operator=(const ScreenSpaceAO& rhs) = delete;
		~ScreenSpaceAO() = default;

		void UpdateSSAOConstants(RenderParam& param, Camera* pCamera);
		void UpdateBlurConstants();

		void PassNormal(RenderParam& param, Scene* target, Camera* camera);
		void PassSSAO(RenderParam& param);
		void PassBlur(RenderParam& param);

		void OnResize(UINT newWidth, UINT newHeight, ID3D12Device* device);
		void BuildResources();
		void BuildDescriptors(DescriptorParam& descriptorParam, ID3D12Resource* depthStencilBuffer);
		void RebuildDescriptors(ID3D12Resource* depthStencilBuffer);
		void BuildRootSignature(ID3D12Device* pDevice);
		void BuildPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature);
		void BuildOffsetVectors();
		void BuildBlurWeights();

	public:
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetAmbientMapGpuSrv() const { return m_ambientMapGpuSrv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetSSAOMapGpuSrv() const { return m_ssaomapGpuSrv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetBlurMapGpuSrv() const { return m_blurMapGpuSrv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetNormalMapGpuSrv() const { return m_normalMapGpuSrv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetDepthMapGpuSrv() const { return m_depthMapGpuSrv; }

	private:
		ID3D12Device* m_device;

		float m_ssaoMapScale;
		UINT m_width;
		UINT m_height;

		ComPtr<ID3D12RootSignature> m_ssaoRootSignature;
		ComPtr<ID3D12RootSignature> m_blurRootSignature;

		ComPtr<ID3D12PipelineState> m_normalPSO;
		ComPtr<ID3D12PipelineState> m_ssaoPSO;
		ComPtr<ID3D12PipelineState> m_blurPSO;

		ComPtr<ID3D12Resource> m_ambientMap;
		ComPtr<ID3D12Resource> m_ssaomap;
		ComPtr<ID3D12Resource> m_blurMap;
		ComPtr<ID3D12Resource> m_normalMap;
		
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_ambientMapCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_ambientMapGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_ambientMapCpuRtv;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_ssaomapCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_ssaomapGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_ssaomapCpuRtv;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blurMapCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_blurMapGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blurMapCpuRtv;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_normalMapCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_normalMapGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_normalMapCpuRtv;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_depthMapCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_depthMapGpuSrv;

		D3D12_VIEWPORT m_ssaoViewport;
		D3D12_RECT m_ssaoScissorRect;

		XMFLOAT4 m_offsets[KERNEL_SIZE];
		FLOAT m_blurWeights[BLUR_SMAPLE];

		std::array<std::unique_ptr<UploadBuffer<SSAOConstants>>, FrameResourceCount> m_constantBuffers;
		std::unique_ptr<UploadBuffer<BlurConstants>> m_blurConstantBuffer;
	};
}