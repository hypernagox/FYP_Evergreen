#pragma once

#include "pch.h"

class GizmoSphereRenderer : public udsdx::Component
{
public:
	void OnDrawGizmos(const udsdx::Camera* target) override;

	float GetRadius() const { return m_radius; }
	void SetRadius(float radius) { m_radius = radius; }
	Vector3 GetOffset() const { return m_offset; }
	void SetOffset(const Vector3& offset) { m_offset = offset; }

private:
	float m_radius = 1.0f;
	Vector3 m_offset = Vector3::Zero;
};