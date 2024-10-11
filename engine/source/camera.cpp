#include "pch.h"
#include "camera.h"
#include "scene.h"
#include "scene_object.h"
#include "transform.h"
#include "frame_resource.h"
#include "core.h"

namespace udsdx
{
	Camera::Camera(const std::shared_ptr<SceneObject>& object) : Component(object)
	{
		for (auto& buffer : m_constantBuffers)
		{
			buffer = std::make_unique<UploadBuffer<CameraConstants>>(INSTANCE(Core)->GetDevice(), 1, true);
		}
	}

	void Camera::PostUpdate(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderCamera(this);
	}

	D3D12_GPU_VIRTUAL_ADDRESS Camera::UpdateConstantBuffer(int frameResourceIndex, float aspect)
	{
		CameraConstants constants;
		Matrix4x4 worldMat = GetTransform()->GetWorldSRTMatrix();
		Matrix4x4 viewMat = GetViewMatrix();
		Matrix4x4 projMat = GetProjMatrix(aspect);
		Matrix4x4 viewProjMat = viewMat * projMat;

		constants.View = viewMat.Transpose();
		constants.Proj = projMat.Transpose();
		constants.ViewProj = viewProjMat.Transpose();
		constants.ViewInverse = viewMat.Invert().Transpose();
		constants.ProjInverse = projMat.Invert().Transpose();
		constants.ViewProjInverse = viewProjMat.Invert().Transpose();
		constants.CameraPosition = Vector4::Transform(Vector4::UnitW, worldMat);

		m_constantBuffers[frameResourceIndex]->CopyData(0, constants);
		return m_constantBuffers[frameResourceIndex]->Resource()->GetGPUVirtualAddress();
	}

	Matrix4x4 Camera::GetViewMatrix() const
	{
		XMMATRIX worldSRTMatrix = XMLoadFloat4x4(&GetSceneObject()->GetTransform()->GetWorldSRTMatrix());
		XMVECTOR eye = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), worldSRTMatrix);
		XMVECTOR at = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), worldSRTMatrix) + eye;
		XMVECTOR up = XMVector4Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), worldSRTMatrix);
		Matrix4x4 m;
		XMStoreFloat4x4(&m, XMMatrixLookAtLH(eye, at, up));
		return m;
	}

	BoundingFrustum Camera::GetViewFrustumWorld(float aspect) const
	{
		BoundingFrustum frustum;
		frustum.CreateFromMatrix(frustum, GetProjMatrix(aspect));
		Matrix4x4 viewInv = GetViewMatrix().Invert();
		frustum.Transform(frustum, XMLoadFloat4x4(&viewInv));
		return frustum;
	}

	void Camera::SetClearColor(const Color& color)
	{
		m_clearColor = color;
	}

	Color Camera::GetClearColor() const
	{
		return m_clearColor;
	}

	CameraPerspective::CameraPerspective(const std::shared_ptr<SceneObject>& object) : Camera(object)
	{
	}

	Matrix4x4 CameraPerspective::GetProjMatrix(float aspect) const
	{
		Matrix4x4 m;
		XMStoreFloat4x4(&m, XMMatrixPerspectiveFovLH(m_fov, aspect, m_near, m_far));
		return m;
	}

	void CameraPerspective::SetFov(float fov)
	{
		m_fov = fov;
	}

	void CameraPerspective::SetNear(float fNear)
	{
		m_near = fNear;
	}

	void CameraPerspective::SetFar(float fFar)
	{
		m_far = fFar;
	}

	float CameraPerspective::GetFov() const
	{
		return m_fov;
	}

	float CameraPerspective::GetNear() const
	{
		return m_near;
	}

	float CameraPerspective::GetFar() const
	{
		return m_far;
	}

	CameraOrthographic::CameraOrthographic(const std::shared_ptr<SceneObject>& object) : Camera(object)
	{
	}

	Matrix4x4 CameraOrthographic::GetProjMatrix(float aspect) const
	{
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, XMMatrixOrthographicLH(aspect * m_radius, m_radius, m_near, m_far));
		return m;
	}

	void CameraOrthographic::SetNear(float fNear)
	{
		m_near = fNear;
	}

	void CameraOrthographic::SetFar(float fFar)
	{
		m_far = fFar;
	}

	void CameraOrthographic::SetRadius(float radius)
	{
		m_radius = radius;
	}

	float CameraOrthographic::GetNear() const
	{
		return m_near;
	}

	float CameraOrthographic::GetFar() const
	{
		return m_far;
	}

	float CameraOrthographic::GetRadius() const
	{
		return m_radius;
	}
}