#pragma once

#include "pch.h"

namespace udsdx
{
	struct ObjectConstants
	{
		Matrix4x4 World = Matrix4x4::Identity;
	};

	struct CameraConstants
	{
		Matrix4x4 View = Matrix4x4::Identity;
		Matrix4x4 Proj = Matrix4x4::Identity;
		Vector4 CameraPosition = Vector4::Zero;
	};

	struct ShadowConstants
	{
		Matrix4x4 LightViewProj[4];
		Matrix4x4 LightViewProjClip[4];
		float ShadowDistance[4];
		float ShadowBias[4];
		Vector3 LightDirection = Vector3::Zero;
	};

	struct PassConstants
	{
		float TotalTime = 0.0f;
	};

	class FrameResource
	{
	public:
		FrameResource(ID3D12Device* device);
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;
		~FrameResource();

	public:
		ID3D12CommandAllocator* GetCommandListAllocator() const;
		UploadBuffer<PassConstants>* GetObjectCB() const;

		UINT64 GetFence() const;
		void SetFence(UINT64 fence);

	private:
		ComPtr<ID3D12CommandAllocator> m_commandListAllocator;
		std::unique_ptr<UploadBuffer<PassConstants>> m_objectCB = nullptr;

		UINT64 m_fence = 0;
	};
}