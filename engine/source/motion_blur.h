#pragma once

#include "pch.h"

namespace udsdx
{
	class Camera;

	class MotionBlur
	{
	public:
		constexpr const static UINT MaxBlurRadius = 20;
		constexpr const static float BlurTimeScale = 0.01f;

	public:
		MotionBlur(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
		MotionBlur(const MotionBlur& rhs) = delete;
		MotionBlur& operator=(const MotionBlur& rhs) = delete;
		~MotionBlur() = default;

		void Pass(RenderParam& param, D3D12_GPU_VIRTUAL_ADDRESS cbvGpu);

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

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_tileMaxCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_tileMaxGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_tileMaxCpuUav;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_tileMaxGpuUav;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_neighborMaxCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_neighborMaxGpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_neighborMaxCpuUav;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_neighborMaxGpuUav;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_sourceCpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_sourceGpuSrv;

		ComPtr<ID3D12Resource> m_tileMaxBuffer;
		ComPtr<ID3D12Resource> m_neighborMaxBuffer;
		ComPtr<ID3D12Resource> m_sourceBuffer;

		ComPtr<ID3D12RootSignature> m_computeRootSignature;
		ComPtr<ID3D12RootSignature> m_rootSignature;

		ComPtr<ID3D12PipelineState> m_tileMaxPso;
		ComPtr<ID3D12PipelineState> m_neighborMaxPso;
		ComPtr<ID3D12PipelineState> m_pso;
	};
}