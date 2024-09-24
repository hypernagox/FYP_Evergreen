#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	class Shader : public ResourceObject
	{
	private:
		// Pipeline State Object:
		// used to define the pipeline state (works like a program in OpenGL)
		ComPtr<ID3D12PipelineState> m_defaultPipelineState;
		ComPtr<ID3D12PipelineState> m_riggedPipelineState;
		ComPtr<ID3D12PipelineState> m_shadowPipelineState;
		ComPtr<ID3D12PipelineState> m_riggedShadowPipelineState;

		std::wstring m_path;

	public:
		Shader(std::wstring_view path);

	public:
		void BuildPipelineState(ID3D12Device* pDevice, ID3D12RootSignature* pRootSignature);

	public:
		ID3D12PipelineState* DefaultPipelineState() const;
		ID3D12PipelineState* RiggedPipelineState() const;
		ID3D12PipelineState* ShadowPipelineState() const;
		ID3D12PipelineState* RiggedShadowPipelineState() const;
	};
}
