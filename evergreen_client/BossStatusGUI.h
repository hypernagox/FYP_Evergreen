#pragma once

#include "pch.h"

class BossStatusGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnActive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	float GetHPFraction() const noexcept { return m_hpFraction; }
	void SetHPFraction(float value) noexcept { m_hpFraction = value; m_hpImpactTime = 0.5f; }

protected:
	std::shared_ptr<udsdx::SceneObject> m_iconObject;
	std::shared_ptr<udsdx::SceneObject> m_panelObject;
	std::shared_ptr<udsdx::SceneObject> m_panelImpactObject;
	std::shared_ptr<udsdx::SceneObject> m_panelFillObject;

	float m_appearTime = 0.0f;
	float m_hpFraction = 1.0f;
	float m_hpImpactFraction = 1.0f;
	float m_hpImpactTime = 0.0f;
};