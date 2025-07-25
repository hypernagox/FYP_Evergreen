#pragma once

#include "pch.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class DeferredRenderer
	{
	public:
		DeferredRenderer(ID3D12Device* device);
		~DeferredRenderer();

		void OnResize(UINT newWidth, UINT newHeight);
		void BuildDescriptors(DescriptorParam& descriptorParam);
		void BuildRootSignature();
		void RebuildDescriptors();
		void BuildResources();

	public:
		void ClearRenderTargets(ID3D12GraphicsCommandList* commandList);
		void SetRenderTargets(ID3D12GraphicsCommandList* commandList);
		void PassBufferPreparation(RenderParam& renderParam);
		void PassBufferPostProcess(RenderParam& renderParam);
		void PassRender(RenderParam& renderParam, D3D12_GPU_VIRTUAL_ADDRESS cbvGpu, const std::vector<ID3D12PipelineState*>& pipelineStates);

	public:
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGBufferSrv(UINT index) const { return m_gBuffersGpuSrv[index]; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetDepthBufferSrv() const { return m_depthBufferGpuSrv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetStencilBufferSrv() const { return m_stencilBufferGpuSrv; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthBufferDsv() const { return m_depthBufferCpuDsv; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetRTVView() const { return m_targetViewCpuRtv; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetRenderTargetSrv() const { return m_targetViewGpuSrv; }
		ID3D12Resource*				  GetRenderTargetResource() const { return m_targetBuffer.Get(); }
		ID3D12RootSignature*		  GetRootSignature() const { return m_renderRootSignature.Get(); }

	public:
		static constexpr UINT NUM_GBUFFERS = 3;

		static constexpr DXGI_FORMAT GBUFFER_FORMATS[NUM_GBUFFERS] = {
			DXGI_FORMAT_R8G8B8A8_UNORM, // RGB: Albedo color
			DXGI_FORMAT_R16G16_SNORM,   // RG: Packed View normal vector
			DXGI_FORMAT_R8G8B8A8_SNORM, // RG: Screenspace motion vector, B: Metallic, A: Roughness
		};

		static constexpr float GBUFFER_CLEAR_VALUES[NUM_GBUFFERS][4] = {
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
		};

		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

	private:
		ID3D12Device* m_device;

		UINT m_width = 0;
		UINT m_height = 0;

		ComPtr<ID3D12RootSignature> m_renderRootSignature;

		// Multiple Render Target (MRT) for deferred rendering
		std::array<ComPtr<ID3D12Resource>, NUM_GBUFFERS> m_gBuffers;
		std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, NUM_GBUFFERS> m_gBuffersCpuSrv;
		std::array<CD3DX12_GPU_DESCRIPTOR_HANDLE, NUM_GBUFFERS> m_gBuffersGpuSrv;
		std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, NUM_GBUFFERS> m_gBuffersCpuRtv;

		// Depth buffer
		ComPtr<ID3D12Resource> m_targetBuffer;
		ComPtr<ID3D12Resource> m_depthBuffer;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_targetViewCpuRtv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_targetViewCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_targetViewGpuSrv;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_depthBufferCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_depthBufferGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_stencilBufferCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_stencilBufferGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_depthBufferCpuDsv;

		std::array<D3D12_RESOURCE_BARRIER, NUM_GBUFFERS + 1> m_gBufferBeginRenderTransitions;
		std::array<D3D12_RESOURCE_BARRIER, NUM_GBUFFERS + 1> m_gBufferEndRenderTransitions;
	};
}