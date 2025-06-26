#include "pch.h"
#include "mono_upload_buffer.h"

namespace udsdx
{
	MonoUploadBuffer::MonoUploadBuffer(ID3D12Device* device) : m_device(device)
	{
		// Initialize with a default size, can be reallocated later
		ReallocateBuffer(m_size);
	}

	MonoUploadBuffer::~MonoUploadBuffer()
	{
	}

	void MonoUploadBuffer::ReallocateBuffer(UINT64 size)
	{
		if (m_resource)
		{
			m_resource.Reset();
		}

		D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)));
	}

	ID3D12Resource* MonoUploadBuffer::PrepareForUpload(UINT64 size)
	{
		// Double the size until it is sufficient
		bool needsReallocation = m_size < size;
		while (m_size < size)
		{
			m_size <<= 1;
		}

		// If the size has changed, reallocate the buffer
		if (needsReallocation)
		{
			ReallocateBuffer(m_size);
		}

		return GetResource();
	}
}