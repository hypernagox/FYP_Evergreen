#pragma once

#include "pch.h"
#include "EventTimer.h"

class DamageCountGUI : public udsdx::Component
{
public:
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void AddCountObject(const Vector3& targetPos, unsigned int damageCount, const int hit_coujnt = 1);

private:
	static std::default_random_engine randomEngine;

	std::deque<std::tuple<float, std::shared_ptr<udsdx::SceneObject>, Vector3, Vector3>> m_countObjects;
	EventTimer m_timer;
	std::unique_ptr<DirectX::SoundEffectInstance> m_soundInstance;
};