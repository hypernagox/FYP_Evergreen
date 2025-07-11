#pragma once

#include "pch.h"

namespace udsdx
{
	class PostProcessBloom
	{
	public:
		PostProcessBloom(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		PostProcessBloom(const PostProcessBloom& rhs) = delete;
		PostProcessBloom& operator=(const PostProcessBloom& rhs) = delete;
		~PostProcessBloom() = default;

		void Pass(RenderParam& param);

		void OnResize(UINT newWidth, UINT newHeight);
		void BuildResources();
		void BuildDescriptors(DescriptorParam& descriptorParam);
		void RebuildDescriptors();
		void BuildRootSignature();
		void BuildPipelineState();

	public:
		float GetUpsampleScale() const { return m_upsampleScale; }
		float GetBloomStrength() const { return m_bloomStrength; }
		float GetExposure() const { return m_exposure; }

		void SetUpsampleScale(float scale) { m_upsampleScale = scale; }
		void SetBloomStrength(float strength) { m_bloomStrength = strength; }
		void SetExposure(float exposure) { m_exposure = exposure; }

	private:
		ID3D12Device* m_device;

		static constexpr UINT NumMipChains = 8;
		static constexpr DXGI_FORMAT BloomFormat = DXGI_FORMAT_R11G11B10_FLOAT;
		static constexpr DXGI_FORMAT DestinationFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr float BloomClearValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		UINT m_width = 0;
		UINT m_height = 0;

		float m_bloomThreshold = 1.0f;
		float m_upsampleScale = 1.0f;
		float m_bloomStrength = 0.4f;
		float m_exposure = 1.3f;

		std::array<ComPtr<ID3D12Resource>, NumMipChains> m_bloomBuffers;
		std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, NumMipChains> m_bloomCpuSrv;
		std::array<CD3DX12_GPU_DESCRIPTOR_HANDLE, NumMipChains> m_bloomGpuSrv;
		std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, NumMipChains> m_bloomCpuRtv;

		std::array<D3D12_VIEWPORT, NumMipChains> m_viewports;
		std::array<D3D12_RECT, NumMipChains> m_scissorRects;

		ComPtr<ID3D12RootSignature> m_rootSignature;

		ComPtr<ID3D12PipelineState> m_psoPrepass;
		ComPtr<ID3D12PipelineState> m_psoDownsample;
		ComPtr<ID3D12PipelineState> m_psoUpsample;
		ComPtr<ID3D12PipelineState> m_psoSynthesize;
	};
}