#include "pch.h"
#include "camera.h"
#include "scene.h"
#include "scene_object.h"
#include "transform.h"
#include "frame_resource.h"
#include "core.h"

namespace udsdx
{
	void Camera::OnInitialize()
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

	D3D12_GPU_VIRTUAL_ADDRESS Camera::UpdateConstantBuffer(int frameResourceIndex, float width, float height)
	{
		float aspect = width / height;

		CameraConstants constants;
		Matrix4x4 worldMat = GetTransform()->GetWorldSRTMatrix(false);
		Matrix4x4 viewMat = GetViewMatrix(false);
		Matrix4x4 projMat = GetProjMatrix(aspect);
		Matrix4x4 viewProjMat = viewMat * projMat;

		constants.View = viewMat.Transpose();
		constants.Proj = projMat.Transpose();
		constants.ViewProj = viewProjMat.Transpose();
		constants.ViewInverse = viewMat.Invert().Transpose();
		constants.ProjInverse = projMat.Invert().Transpose();
		constants.ViewProjInverse = viewProjMat.Invert().Transpose();
		constants.PrevViewProj = m_prevViewProjMatrix.Transpose();
		constants.CameraPosition = Vector4::Transform(Vector4::UnitW, worldMat);
		constants.RenderTargetSize = Vector2(width, height);

		m_prevViewProjMatrix = viewProjMat;

		m_constantBuffers[frameResourceIndex]->CopyData(0, constants);
		return m_constantBuffers[frameResourceIndex]->Resource()->GetGPUVirtualAddress();
	}

	Matrix4x4 Camera::GetViewMatrix(bool validate) const
	{
		XMMATRIX worldSRTMatrix = XMLoadFloat4x4(&GetSceneObject()->GetTransform()->GetWorldSRTMatrix(validate));
		XMVECTOR eye = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), worldSRTMatrix);
		XMVECTOR at = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), worldSRTMatrix) + eye;
		XMVECTOR up = XMVector4Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), worldSRTMatrix);
		Matrix4x4 m;
		XMStoreFloat4x4(&m, XMMatrixLookAtLH(eye, at, up));
		return m;
	}

	void Camera::SetClearColor(const Color& color)
	{
		m_clearColor = color;
	}

	Color Camera::GetClearColor() const
	{
		return m_clearColor;
	}

	const Vector3 Camera::ToViewPosition(const Vector3& worldPosition) const
	{
		return Vector3::Transform(worldPosition, GetViewMatrix(true));
	}

	const Vector2 Camera::ToScreenPosition(const Vector3& worldPosition) const
	{
		const int screenWidth = INSTANCE(Core)->GetClientWidth();
		const int screenHeight = INSTANCE(Core)->GetClientHeight();

		Vector3 screenPos = Vector3::Transform(worldPosition, GetViewMatrix(true) * GetProjMatrix(static_cast<float>(screenWidth) / screenHeight));
		screenPos.x = (screenPos.x + 1.0f) * 0.5f;
		screenPos.y = (1.0f - screenPos.y) * 0.5f;

		return Vector2(screenPos.x * screenWidth, screenPos.y * screenHeight);
	}

	Matrix4x4 CameraPerspective::GetProjMatrix(float aspect) const
	{
		Matrix4x4 m;
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, aspect, m_near, m_far);
		projectionMatrix *= XMMatrixTranslation(m_clipOffset.x, m_clipOffset.y, 0.0f);
		XMStoreFloat4x4(&m, projectionMatrix);
		return m;
	}

	std::unique_ptr<BoundingCamera> CameraPerspective::GetViewFrustumWorld(float aspect) const
	{
		return std::make_unique<BoundingCameraPerspective>(GetViewMatrix(false), GetProjMatrix(aspect));
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

	Matrix4x4 CameraOrthographic::GetProjMatrix(float aspect) const
	{
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, XMMatrixOrthographicLH(aspect * m_radius, m_radius, m_near, m_far));
		return m;
	}

	std::unique_ptr<BoundingCamera> CameraOrthographic::GetViewFrustumWorld(float aspect) const
	{
		return std::make_unique<BoundingCameraOrthographic>(GetViewMatrix(false), m_radius * 2.0f * aspect, m_radius * 2.0f, m_near, m_far);
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