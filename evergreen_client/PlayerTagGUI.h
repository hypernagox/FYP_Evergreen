#pragma once

#include "pch.h"

class PlayerTagGUI : public udsdx::Component
{
public:
	PlayerTagGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetTargetPosition(const Vector3& targetPos) { m_targetPos = targetPos; }

private:
	Vector3 m_targetPos = Vector3::Zero;

	std::shared_ptr<udsdx::SceneObject> m_panelObject;
	std::shared_ptr<udsdx::SceneObject> m_nameObject;
};