#pragma once

#include "pch.h"

class DamageCountGUI : public udsdx::Component
{
public:
	DamageCountGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void AddCountObject(const Vector3& targetPos, unsigned int damageCount);

private:
	static std::default_random_engine randomEngine;

	std::deque<std::tuple<float, std::shared_ptr<udsdx::SceneObject>, Vector3, Vector3>> m_countObjects;
};