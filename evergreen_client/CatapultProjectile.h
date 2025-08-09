#pragma once

#include "pch.h"

class CatapultProjectile : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void ShootProjectile(const udsdx::Vector3& fromPosition, const std::shared_ptr<udsdx::SceneObject>& targetObj, float duration);

private:
	float m_duration = 0.0f;
	float m_shootElapsedTime = 0.0f;
	Vector3 m_fromPosition = Vector3::Zero;
	std::shared_ptr<udsdx::SceneObject> m_projectileObj;
	std::shared_ptr<udsdx::SceneObject> m_targetObj;
};