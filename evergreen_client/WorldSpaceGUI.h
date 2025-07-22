#pragma once

#include "pch.h"

class WorldSpaceGUI : public udsdx::Component
{
public:
	void PostUpdate(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetTargetObject(const std::shared_ptr<udsdx::SceneObject>& targetObject);
	void SetSourceTransform(udsdx::Transform* sourceTransform);
	void SetWorldOffset(const Vector3& offset);
	void SetScreenOffset(const Vector3& offset);
	void SetViewFar(float viewFar);
	void SetAngleRange(float degrees);

private:
    std::shared_ptr<udsdx::SceneObject> m_targetObject = nullptr;
	udsdx::Transform* m_sourceTransform = nullptr;
	Vector3 m_worldOffset = Vector3::Zero;
	Vector3 m_screenOffset = Vector3::Zero;
    float m_viewFar = 1000.0f;
	float m_angleRange = 0.0f; // cos(theta) for dot product
};