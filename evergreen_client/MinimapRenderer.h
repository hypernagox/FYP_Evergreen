#pragma once

#include "pch.h"

class MinimapRenderer
{
public:
	MinimapRenderer(ID3D12Device* device, UINT width, UINT height);
	~MinimapRenderer();

	void BuildRootSignature();
	void BuildResources();
	void BuildDescriptors(udsdx::DescriptorParam& descriptorParam);
	void BuildPipelineStateObject();
	void SetViewMatrix(const udsdx::Vector3& position, const udsdx::Vector3& forward);

public:
	void PassRender(udsdx::RenderParam& renderParam);

public:
	udsdx::Texture* GetRenderTargetTexture() const { return m_renderTargetTexture.get(); }
	void SetMinimapMesh(udsdx::Mesh* mesh) { m_minimapMesh = mesh; }

public:
	static constexpr DXGI_FORMAT RENDER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
	static constexpr float RENDER_CLEAR_VALUE[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

private:
	ID3D12Device* m_device;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	ComPtr<ID3D12Resource> m_renderTarget;
	ComPtr<ID3D12Resource> m_depthStencil;

	D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvCpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvCpuHandle;

	UINT m_width = 0;
	UINT m_height = 0;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	udsdx::Mesh* m_minimapMesh = nullptr;
	std::unique_ptr<udsdx::Texture> m_renderTargetTexture;

	udsdx::Matrix4x4 m_viewMatrix;
};

