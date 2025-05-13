#pragma once

#include "pch.h"

namespace udsdx
{
	class Transform
	{
	public:
		Transform() = default;
		~Transform() = default;

	public:
		void SetParent(Transform* parent);
		Transform* GetParent() const;

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

		// Validate the local SRT matrix.
		// If the local matrix is dirty, the local SRT matrix is recalculated.
		// The return value is true if the local matrix was dirty; you can use the return value to determine whether the children need to be recalculated.
		bool ValidateLocalSRTMatrix();

		// Validate the world SRT matrix.
		// If the world matrix is dirty, the world SRT matrix is recalculated.
		// Calling this function assumes the world SRT matrix of the parent is already calculated.
		void ValidateWorldSRTMatrix();

		// Validate both the local and the world SRT matrices.
		// This function traverses the all parents of the transform and validates the world SRT matrix of each parent.
		// This provides the most accurate result, but may be slow in some cases.
		// It is recommended to use this function only when you want to know the world transform of the other Transform once.
		void ValidateMatrixRecursive();

	protected:
		Transform*	m_parent = nullptr;

		Vector3		m_position = Vector3::Zero;
		Quaternion	m_rotation = Quaternion::Identity;
		Vector3		m_scale = Vector3::One;

		Matrix4x4	m_localSRTMatrix = Matrix4x4::Identity;
		Matrix4x4	m_worldSRTMatrix = Matrix4x4::Identity;

		bool		m_isLocalMatrixDirty = true;
	};
}