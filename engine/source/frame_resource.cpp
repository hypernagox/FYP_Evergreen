#include "pch.h"
#include "frame_resource.h"

namespace udsdx
{
	FrameResource::FrameResource(ID3D12Device* device)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandListAllocator)
		));
		m_objectCB = std::make_unique<UploadBuffer<PassConstants>>(device, 1, true);
	}

	FrameResource::~FrameResource()
	{

	}

	ID3D12CommandAllocator* FrameResource::GetCommandListAllocator() const
	{
		return m_commandListAllocator.Get();
	}

	UploadBuffer<PassConstants>* FrameResource::GetObjectCB() const
	{
		return m_objectCB.get();
	}

	UINT64 FrameResource::GetFence() const
	{
		return m_fence;
	}

	void FrameResource::SetFence(UINT64 fence)
	{
		m_fence = fence;
	}
}