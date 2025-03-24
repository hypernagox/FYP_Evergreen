#pragma once

#include "pch.h"

namespace udsdx
{
	enum class Pivot
	{
		TopLeft,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BottomLeft,
		BottomCenter,
		BottomRight
	};

	class RectTransform
	{
	public:
		RectTransform();
		~RectTransform();

	public:
		void SetParent(RectTransform* parent);
		RectTransform* GetParent() const;

		void SetLocalPosition(const Vector3& position);
		void SetLocalRotation(const Quaternion& rotation);
		void SetLocalScale(const Vector3& scale);

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

		Matrix4x4 GetLocalSRTMatrix() const;
		Matrix4x4 GetWorldSRTMatrix() const;

		bool ValidateLocalSRTMatrix();
		void ValidateWorldSRTMatrix();
		void ValidateMatrixRecursive();

	protected:
		RectTransform* m_parent;

		Vector2		m_position;
		Quaternion	m_rotation;
		Vector2		m_scale;

		Matrix4x4	m_localSRTMatrix;
		Matrix4x4	m_worldSRTMatrix;

		bool		m_isLocalMatrixDirty;
	};
}