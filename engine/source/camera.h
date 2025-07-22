#pragma once

#include "pch.h"
#include "component.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;

	class BoundingCamera
	{
	public:
		virtual ContainmentType Contains(const BoundingBox& box) const = 0;
	};

	class BoundingCameraOrthographic : public BoundingCamera
	{
	private:
		BoundingOrientedBox m_cameraOrientedBox;

	public:
		BoundingCameraOrthographic(Matrix4x4 viewMatrix, float width, float height, float nearPlane, float farPlane)
		{
			m_cameraOrientedBox = BoundingOrientedBox(Vector3::Forward * (nearPlane + farPlane) * 0.5f, Vector3(width * 0.5f, height * 0.5f, (farPlane - nearPlane)), Quaternion::Identity);
			m_cameraOrientedBox.Transform(m_cameraOrientedBox, viewMatrix.Invert());
		}

		ContainmentType Contains(const BoundingBox& box) const override
		{
			return m_cameraOrientedBox.Contains(box);
		}
	};

	class BoundingCameraPerspective : public BoundingCamera
	{
	private:
		BoundingFrustum m_frustum;

	public:
		BoundingCameraPerspective(Matrix4x4 viewMatrix, Matrix4x4 projMatrix)
		{
			m_frustum = BoundingFrustum(projMatrix);
			m_frustum.Transform(m_frustum, viewMatrix.Invert());
		}

		ContainmentType Contains(const BoundingBox& box) const override
		{
			return m_frustum.Contains(box);
		}
	};

	class Camera : public Component
	{
	public:
		virtual void OnInitialize() override;
		virtual void PostUpdate(const Time& time, Scene& scene) override;
		D3D12_GPU_VIRTUAL_ADDRESS UpdateConstantBuffer(int frameResourceIndex, float width, float height);

	public:
		virtual Matrix4x4 GetViewMatrix(bool validate = true) const;
		virtual Matrix4x4 GetProjMatrix(float aspect) const = 0;
		virtual std::unique_ptr<BoundingCamera> GetViewFrustumWorld(float aspect) const = 0;

		void SetClearColor(const Color& color);
		Color GetClearColor() const;

		void SetClipOffset(const Vector2& offset) { m_clipOffset = offset; }
		Vector2 GetClipOffset() const { return m_clipOffset; }

		const Vector3 ToViewPosition(const Vector3& worldPosition) const;
		const Vector2 ToScreenPosition(const Vector3& worldPosition) const;

	protected:
		std::array<std::unique_ptr<UploadBuffer<CameraConstants>>, FrameResourceCount> m_constantBuffers;

		Color m_clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
		Matrix4x4 m_prevViewProjMatrix = Matrix4x4::Identity;
		Vector2 m_clipOffset = Vector2(0.0f, 0.0f);
	};

	class CameraPerspective : public Camera
	{
	public:
		virtual Matrix4x4 GetProjMatrix(float aspect) const override;
		virtual std::unique_ptr<BoundingCamera> GetViewFrustumWorld(float aspect) const override;

	public:
		void SetFov(float fov);
		void SetNear(float fNear);
		void SetFar(float fFar);

		float GetFov() const;
		float GetNear() const;
		float GetFar() const;

	protected:
		float m_fov = PI / 3;
		float m_near = 0.1f;
		float m_far = 1000.0f;
	};

	class CameraOrthographic : public Camera
	{
	public:
		virtual Matrix4x4 GetProjMatrix(float aspect) const override;
		virtual std::unique_ptr<BoundingCamera> GetViewFrustumWorld(float aspect) const override;

	public:
		void SetNear(float fNear);
		void SetFar(float fFar);
		void SetRadius(float radius);

		float GetNear() const;
		float GetFar() const;
		float GetRadius() const;

	protected:
		float m_near = 0.1f;
		float m_far = 1000.0f;
		float m_radius = 1.0f;
	};
}