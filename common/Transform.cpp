#include "pch.h"
#include "Transform.h"

namespace Common
{
	using namespace DirectX;

	Transform::Transform()
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
		m_matrixDirty = true;
	}

	void Transform::SetLocalRotation(const Quaternion& rotation)
	{
		m_rotation = rotation;
		m_matrixDirty = true;
	}

	void Transform::SetLocalScale(const Vector3& scale)
	{
		m_scale = scale;
		m_matrixDirty = true;
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
		return Vector3::Transform(Vector3::Zero, m_worldSRTMatrix);
	}

	Quaternion Transform::GetWorldRotation()
	{
		ValidateMatrixRecursive();
		return Quaternion::CreateFromRotationMatrix(m_worldSRTMatrix);
	}

	Matrix Transform::GetLocalSRTMatrix()
	{
		ValidateLocalSRTMatrix();
		return m_localSRTMatrix;
	}

	Matrix Transform::GetWorldSRTMatrix()
	{
		ValidateMatrixRecursive();
		return m_worldSRTMatrix;
	}

	// Validate the local SRT matrix.
	// If the local matrix is dirty, the local SRT matrix is recalculated.
	// The return value is true if the local matrix was dirty; you can use the return value to determine whether the children need to be recalculated.
	bool Transform::ValidateLocalSRTMatrix()
	{
		if (!m_matrixDirty)
		{
			return false;
		}

		// All properties of the transform are converted to XMVECTOR without an explicit conversion.
		XMVECTOR t = XMLoadFloat3(&m_position);
		XMVECTOR r = XMLoadFloat4(&m_rotation);
		XMVECTOR s = XMLoadFloat3(&m_scale);
		XMMATRIX m = XMMatrixAffineTransformation(s, XMVectorZero(), r, t);

		// Apply the local SRT matrix.
		XMStoreFloat4x4(&m_localSRTMatrix, m);

		m_matrixDirty = false;
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