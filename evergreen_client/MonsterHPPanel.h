#pragma once

#include "pch.h"

using namespace udsdx;

class MonsterHPPanel : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnAttach() override;
	void OnDetach() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	float GetHPFraction() const noexcept { return m_hpFraction; }
	void SetHPFraction(float value) noexcept { m_hpFraction = value; m_hpImpactTime = 0.5f; }
	void SetText(std::wstring_view text);

protected:
	std::shared_ptr<udsdx::SceneObject> m_panelObject;
	std::shared_ptr<udsdx::SceneObject> m_levelObject;
	std::shared_ptr<udsdx::SceneObject> m_textObject;
	std::shared_ptr<udsdx::SceneObject> m_hpBarImpactObject;
	std::shared_ptr<udsdx::SceneObject> m_hpBarObject;

	float m_hpFraction = 1.0f;
	float m_hpImpactFraction = 1.0f;
	float m_hpImpactTime = 0.0f;
};