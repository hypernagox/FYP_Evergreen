#pragma once

#include "pch.h"

class PlayerTagGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetTargetPosition(const Vector3& targetPos) { m_targetPos = targetPos; }

private:
	Vector3 m_targetPos = Vector3::Zero;

	std::shared_ptr<udsdx::SceneObject> m_nameObject;
};