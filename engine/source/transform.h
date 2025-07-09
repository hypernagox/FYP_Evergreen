#pragma once

#include "pch.h"

namespace udsdx
{
	class Transform
	{
		friend class SceneObject;

	public:
		Transform() = default;
		~Transform() = default;

	public:
		void SetLocalPosition(const Vector3& position);
		void SetLocalPosition(float x, float y, float z);
		void SetLocalRotation(const Quaternion& rotation);
		void SetLocalScale(const Vector3& scale);
		void SetLocalScale(float x, float y, float z);
		void SetLocalScale(float scale);

		void SetLocalPositionX(float x);
		void SetLocalPositionY(float y);
		void SetLocalPositionZ(float z);

		void Translate(const Vector3& translation);
		void Rotate(const Quaternion& rotation);

		Vector3 GetLocalPosition() const;
		Quaternion GetLocalRotation() const;
		Vector3 GetLocalScale() const;

		Vector3 GetWorldPosition();
		Quaternion GetWorldRotation();

		Matrix4x4 GetLocalSRTMatrix();
		Matrix4x4 GetWorldSRTMatrix(bool forceValidate = true);

		void RecalculateLocalSRTMatrix();
		void RecalculateWorldSRTMatrix();

		// Validate both the local and the world SRT matrix.
		// If the m_isLocalMatrixDirty is true, the local SRT matrix is recalculated.
		// If the m_isWorldMatrixDirty is true, the world SRT matrix is recalculated. And the children are set to dirty as propagation.
		// Calling this function assumes the world SRT matrix of the parent is already calculated.
		void ValidateSRTMatrices();

		// Validate both the local and the world SRT matrices.
		// This function traverses the all parents of the transform and validates the world SRT matrix of each parent.
		// This provides the most accurate result, but may be slow in some cases.
		// It is recommended to use this function only when you want to know the world transform of the other Transform once.
		void ValidateMatrixRecursive();

	protected:
		Transform*	m_parent = nullptr;
		std::vector<Transform*> m_children;

		Vector3		m_position = Vector3::Zero;
		Quaternion	m_rotation = Quaternion::Identity;
		Vector3		m_scale = Vector3::One;

		Matrix4x4	m_localSRTMatrix = Matrix4x4::Identity;
		Matrix4x4	m_worldSRTMatrix = Matrix4x4::Identity;

		bool        m_isLocalMatrixDirty = true;
		bool        m_isWorldMatrixDirty = true;
	};
}