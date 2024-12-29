#pragma once
#include "pch.h"

namespace Common
{
	using namespace DirectX::SimpleMath;

	class Transform
	{
	public:
		Transform();

	public:
		void SetParent(Transform* parent);
		Transform* GetParent() const;

		void SetLocalPosition(const Vector3& position);
		void SetLocalRotation(const Quaternion& rotation);
		void SetLocalScale(const Vector3& scale);

		Vector3 GetLocalPosition() const;
		Quaternion GetLocalRotation() const;
		Vector3 GetLocalScale() const;

		Vector3 GetWorldPosition();
		Quaternion GetWorldRotation();

		Matrix GetLocalSRTMatrix();
		Matrix GetWorldSRTMatrix();

		bool ValidateLocalSRTMatrix();
		void ValidateWorldSRTMatrix();
		void ValidateMatrixRecursive();

	protected:
		Transform*	m_parent = nullptr;

		Vector3		m_position = Vector3::Zero;
		Quaternion	m_rotation = Quaternion::Identity;
		Vector3		m_scale = Vector3::One;

		Matrix		m_localSRTMatrix = Matrix::Identity;
		Matrix		m_worldSRTMatrix = Matrix::Identity;

		bool		m_matrixDirty = true;
	};
}