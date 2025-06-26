#include "pch.h"
#include "mesh_base.h"
#include "core.h"

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

		// Create the GPU buffer for vertex buffer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferByteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_vertexBufferGPU.GetAddressOf())));

		// Create the GPU buffer for index buffer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferByteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_indexBufferGPU.GetAddressOf())));

		UINT64 uploadBufferSize = m_vertexBufferByteSize + m_indexBufferByteSize;
		ID3D12Resource* uploadBuffer = INSTANCE(Core)->GetMonoUploadBuffer()->PrepareForUpload(uploadBufferSize);

		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA vbSubResourceData = {};
		vbSubResourceData.pData = m_vertexBufferCPU->GetBufferPointer();
		vbSubResourceData.RowPitch = m_vertexBufferByteSize;
		vbSubResourceData.SlicePitch = vbSubResourceData.RowPitch;

		D3D12_SUBRESOURCE_DATA ibSubResourceData = {};
		ibSubResourceData.pData = m_indexBufferCPU->GetBufferPointer();
		ibSubResourceData.RowPitch = m_indexBufferByteSize;
		ibSubResourceData.SlicePitch = ibSubResourceData.RowPitch;

		INSTANCE(Core)->PrepareDirectCommandList();

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources<1>(commandList, m_vertexBufferGPU.Get(), uploadBuffer, 0, 0, 1, &vbSubResourceData);
		UpdateSubresources<1>(commandList, m_indexBufferGPU.Get(), uploadBuffer, m_vertexBufferByteSize, 0, 1, &ibSubResourceData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		INSTANCE(Core)->ExecuteAndFlushDirectCommandList();
	}
}