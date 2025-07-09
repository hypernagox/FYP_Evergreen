#include "pch.h"
#include "transform.h"

namespace udsdx
{
	unsigned long long g_localMatrixRecalculateCounter = 0;
	unsigned long long g_worldMatrixRecalculateCounter = 0;

	void Transform::SetLocalPosition(const Vector3& position)
	{
		m_position = position;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalPosition(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
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

	void Transform::SetLocalScale(float x, float y, float z)
	{
		m_scale.x = x;
		m_scale.y = y;
		m_scale.z = z;
		m_isLocalMatrixDirty = true;
	}

	void Transform::SetLocalScale(float scale)
	{
		m_scale.x = scale;
		m_scale.y = scale;
		m_scale.z = scale;
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
		m_rotation = Quaternion::Concatenate(m_rotation, rotation);
		m_rotation.Normalize();
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
		Vector3 xBasis(m_worldSRTMatrix._11, m_worldSRTMatrix._12, m_worldSRTMatrix._13);
		Vector3 yBasis(m_worldSRTMatrix._21, m_worldSRTMatrix._22, m_worldSRTMatrix._23);
		Vector3 zBasis(m_worldSRTMatrix._31, m_worldSRTMatrix._32, m_worldSRTMatrix._33);
		xBasis.Normalize();
		yBasis.Normalize();
		zBasis.Normalize();
		Matrix4x4 worldRotationMatrix(xBasis, yBasis, zBasis); // Create a rotation matrix from the orthogonal vectors.
		return Quaternion::CreateFromRotationMatrix(Matrix4x4(worldRotationMatrix));
	}

	Matrix4x4 Transform::GetLocalSRTMatrix()
	{
		if (m_isLocalMatrixDirty)
		{
			RecalculateLocalSRTMatrix();
			m_isLocalMatrixDirty = false;
			m_isWorldMatrixDirty = true;
		}
		return m_localSRTMatrix;
	}

	Matrix4x4 Transform::GetWorldSRTMatrix(bool forceValidate)
	{
		if (forceValidate)
		{
			ValidateMatrixRecursive();
		}
		return m_worldSRTMatrix;
	}

	void Transform::RecalculateLocalSRTMatrix()
	{
		g_localMatrixRecalculateCounter++;

		// All properties of the transform are converted to XMVECTOR without an explicit conversion.
		XMVECTOR t = XMLoadFloat3(&m_position);
		XMVECTOR r = XMLoadFloat4(&m_rotation);
		XMVECTOR s = XMLoadFloat3(&m_scale);
		XMMATRIX m = XMMatrixAffineTransformation(s, XMVectorZero(), r, t);

		// Apply the local SRT matrix.
		XMStoreFloat4x4(&m_localSRTMatrix, m);
	}

	void Transform::RecalculateWorldSRTMatrix()
	{
		g_worldMatrixRecalculateCounter++;

		// If the parent is null, the transform is the root of the hierarchy.
		// This can be either the root of the scene or the root of the pre-constructed hierarchy.
		if (m_parent == nullptr)
		{
			m_worldSRTMatrix = m_localSRTMatrix;
			return;
		}

		XMMATRIX ml = XMLoadFloat4x4(&m_localSRTMatrix);
		XMMATRIX mp = XMLoadFloat4x4(&m_parent->m_worldSRTMatrix);
		XMMATRIX m = XMMatrixMultiply(ml, mp);

		// Apply the world SRT matrix.
		XMStoreFloat4x4(&m_worldSRTMatrix, m);
	}

	void Transform::ValidateSRTMatrices()
	{
		if (m_isLocalMatrixDirty)
		{
			RecalculateLocalSRTMatrix();
			m_isLocalMatrixDirty = false;
			m_isWorldMatrixDirty = true;
		}

		if (m_isWorldMatrixDirty)
		{
			RecalculateWorldSRTMatrix();
			m_isWorldMatrixDirty = false;
			for (Transform* child : m_children)
			{
				// Mark all children as dirty.
				child->m_isWorldMatrixDirty = true;
			}
		}
	}

	void Transform::ValidateMatrixRecursive()
	{
		static std::stack<Transform*> stack;
		Transform* current = this;
		while (current != nullptr)
		{
			stack.emplace(current);
			current = current->m_parent;
		}

		while (!stack.empty())
		{
			current = stack.top();
			stack.pop();

			// Validate the SRT matrices of the current transform.
			ValidateSRTMatrices();
		}
	}
}