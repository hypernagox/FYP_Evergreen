#pragma once

#include "pch.h"

namespace udsdx
{
	class MonoUploadBuffer
	{
	public:
		MonoUploadBuffer(ID3D12Device* device);
		MonoUploadBuffer(const MonoUploadBuffer& rhs) = delete;
		MonoUploadBuffer& operator=(const MonoUploadBuffer& rhs) = delete;
		~MonoUploadBuffer();

	public:
		void ReallocateBuffer(UINT64 size);
		ID3D12Resource* PrepareForUpload(UINT64 size);
		ID3D12Resource* GetResource() const { return m_resource.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

		UINT64 m_size = 1 << 20; // 1 MB default size
	};
}