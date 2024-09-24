#include "pch.h"
#include "mesh_base.h"

namespace udsdx
{
	MeshBase::MeshBase() : ResourceObject()
	{
	}

	D3D12_VERTEX_BUFFER_VIEW MeshBase::VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = m_vertexByteStride;
		vbv.SizeInBytes = m_vertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW MeshBase::IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = INDEX_FORMAT;
		ibv.SizeInBytes = m_indexBufferByteSize;

		return ibv;
	}

	const std::vector<Submesh>& MeshBase::GetSubmeshes() const
	{
		return m_submeshes;
	}

	const BoundingBox& MeshBase::GetBounds() const
	{
		return m_bounds;
	}

	void MeshBase::UploadBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	{
		// Make sure buffers are uploaded to the CPU.
		assert(m_vertexBufferCPU != nullptr);
		assert(m_indexBufferCPU != nullptr);

		m_vertexBufferGPU = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_vertexBufferCPU->GetBufferPointer(),
			m_vertexBufferByteSize,
			m_vertexBufferUploader
		);

		m_indexBufferGPU = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_indexBufferCPU->GetBufferPointer(),
			m_indexBufferByteSize,
			m_indexBufferUploader
		);
	}

	void MeshBase::DisposeUploaders()
	{
		m_vertexBufferUploader = nullptr;
		m_indexBufferUploader = nullptr;

		m_vertexBufferCPU = nullptr;
		m_indexBufferCPU = nullptr;
	}
}