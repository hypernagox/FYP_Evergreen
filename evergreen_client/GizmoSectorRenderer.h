#pragma once

#include "pch.h"

class GizmoSectorRenderer : public udsdx::Component
{
public:
	void OnDrawGizmos(const udsdx::Camera* target) override;

	void SetRadius(float radius) { m_radius = radius; }
	void SetAngle(float angle) { m_angle = angle; }

private:
	float m_radius = 1.0f;
	float m_angle = 90.0f;
};