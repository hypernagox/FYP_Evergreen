#pragma once

#include "pch.h"

namespace udsdx
{
	class PostProcessFXAA
	{
	public:
		PostProcessFXAA(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		PostProcessFXAA(const PostProcessFXAA& rhs) = delete;
		PostProcessFXAA& operator=(const PostProcessFXAA& rhs) = delete;
		~PostProcessFXAA() = default;

		void Pass(RenderParam& param);

		void OnResize(UINT newWidth, UINT newHeight);
		void BuildResources();
		void BuildDescriptors(DescriptorParam& descriptorParam);
		void RebuildDescriptors();
		void BuildRootSignature();
		void BuildPipelineState();

	private:
		ID3D12Device* m_device;

		UINT m_width = 0;
		UINT m_height = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_sourceCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_sourceGpuSrv;

		ComPtr<ID3D12Resource> m_sourceBuffer;

		ComPtr<ID3D12RootSignature> m_rootSignature;

		ComPtr<ID3D12PipelineState> m_pso;
	};
}