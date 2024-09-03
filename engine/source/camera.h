#pragma once

#include "pch.h"
#include "component.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;

	class Camera : public Component
	{
	public:
		Camera(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Update(const Time& time, Scene& scene) override;

	public:
		virtual Matrix4x4 GetViewMatrix() const;
		virtual Matrix4x4 GetProjMatrix(float aspect) const = 0;
		BoundingFrustum GetViewFrustumWorld(float aspect) const;

		void SetClearColor(const Color& color);
		Color GetClearColor() const;

	protected:
		std::array<std::unique_ptr<UploadBuffer<CameraConstants>>, FrameResourceCount> m_constantBuffers;

		Color m_clearColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
	};

	class CameraPerspective : public Camera
	{
	public:
		CameraPerspective(const std::shared_ptr<SceneObject>& object);

	public:
		virtual Matrix4x4 GetProjMatrix(float aspect) const override;

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
		CameraOrthographic(const std::shared_ptr<SceneObject>& object);

	public:
		virtual Matrix4x4 GetProjMatrix(float aspect) const override;

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