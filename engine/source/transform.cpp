#include "pch.h"
#include "transform.h"

namespace udsdx
{
	Transform::Transform()
	{
		m_parent = nullptr;

		m_position = Vector3::Zero;
		m_rotation = Quaternion::Identity;
		m_scale = Vector3::One;

		m_localSRTMatrix = Matrix4x4::Identity;
		m_worldSRTMatrix = Matrix4x4::Identity;

		m_isLocalMatrixDirty = true;
	}

	Transform::~Transform()
	{

	}

	void Transform::SetParent(Transform* parent)
	{
		m_parent = parent;
	}

	Transform* Transform::GetParent() const
	{
		return m_parent;
	}

	void Transform::SetLocalPosition(const Vector3& position)
	{
		m_position = position;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalRotation(const Quaternion& rotation)
	{
		m_rotation = rotation;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalScale(const Vector3& scale)
	{
		m_scale = scale;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalPositionX(float x)
	{
		m_position.x = x;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalPositionY(float y)
	{
		m_position.y = y;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalPositionZ(float z)
	{
		m_position.z = z;
		m_isLocalMatrixDirty = true;
	}

	void Transform::Translate(const Vector3& translation)
	{
		m_position += translation;
		m_isLocalMatrixDirty = true;
	}

	void Transform::Rotate(const Quaternion& rotation)
	{
		m_rotation *= rotation;
		m_isLocalMatrixDirty = true;
	}

	Vector3 Transform::GetLocalPosition() const
	{
		return m_position;
	}

	Quaternion Transform::GetLocalRotation() const
	{
		return m_rotation;
	}

	Vector3 Transform::GetLocalScale() const
	{
		return m_scale;
	}

	Vector3 Transform::GetWorldPosition()
	{
		ValidateMatrixRecursive();
		// Get the translation part of the world matrix.
		return Vector3(m_worldSRTMatrix._41, m_worldSRTMatrix._42, m_worldSRTMatrix._43);
	}

	Quaternion Transform::GetWorldRotation()
	{
		ValidateMatrixRecursive();
		// Caution: The matrix must be orthogonal to get the correct quaternion.
		return Quaternion::CreateFromRotationMatrix(Matrix4x4(m_worldSRTMatrix));
	}

	Matrix4x4 Transform::GetLocalSRTMatrix() const
	{
		return m_localSRTMatrix;
	}

	Matrix4x4 Transform::GetWorldSRTMatrix() const
	{
		return m_worldSRTMatrix;
	}

	bool Transform::ValidateLocalSRTMatrix()
	{
		if (!m_isLocalMatrixDirty)
		{
			return false;
		}

		// All properties of the transform are converted to XMVECTOR without an explicit conversion.
		XMMATRIX m = XMMatrixScalingFromVector(m_scale);
		m = XMMatrixMultiply(m, XMMatrixRotationQuaternion(m_rotation));
		m = XMMatrixMultiply(m, XMMatrixTranslationFromVector(m_position));

		// Apply the local SRT matrix.
		XMStoreFloat4x4(&m_localSRTMatrix, m);

		m_isLocalMatrixDirty = false;
		return true;
	}

	void Transform::ValidateWorldSRTMatrix()
	{
		// If the parent is null, the transform is the root of the hierarchy.
		// This can be either the root of the scene or the root of the pre-constructed hierarchy.
		if (m_parent == nullptr)
		{
			m_worldSRTMatrix = m_localSRTMatrix;
			return;
		}

		XMMATRIX m = XMLoadFloat4x4(&m_localSRTMatrix);
		m = XMMatrixMultiply(m, m_parent->m_worldSRTMatrix);

		// Apply the world SRT matrix.
		XMStoreFloat4x4(&m_worldSRTMatrix, m);
	}

	void Transform::ValidateMatrixRecursive()
	{
		if (m_parent != nullptr)
		{
			m_parent->ValidateMatrixRecursive();
		}
		ValidateLocalSRTMatrix();
		ValidateWorldSRTMatrix();
	}
}