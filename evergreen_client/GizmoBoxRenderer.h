#pragma once

#include "pch.h"

class GizmoBoxRenderer : public udsdx::Component
{
public:
	void OnDrawGizmos(const udsdx::Camera* target) override;

	Vector3 GetSize() const { return m_size; }
	void SetSize(const Vector3& size) { m_size = size; }
	Vector3 GetOffset() const { return m_offset; }
	void SetOffset(const Vector3& offset) { m_offset = offset; }

private:
	Vector3 m_size = Vector3::One;
	Vector3 m_offset = Vector3::Zero;
};

