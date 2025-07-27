#pragma once

#include "pch.h"

class PlayerStatusGUI : public udsdx::Component
{
public:
	void OnInitialize() override;

	void SetMaxHealth(int value);
	void SetCurrentHealth(int value);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void IncHP(const int hp_inc_val) { 
		const auto after_hp = m_currentHealth + hp_inc_val;
		if (after_hp > m_maxHealth)return;
		SetCurrentHealth(after_hp);
	}
	void UseSkill(const int skillIndex);
private:
	static constexpr int NumSkill = 3;

	std::shared_ptr<udsdx::SceneObject> m_healthBackground;
	std::shared_ptr<udsdx::SceneObject> m_healthFill;
	std::shared_ptr<udsdx::SceneObject> m_textObj;
	udsdx::GUIText* m_textRenderer;

	std::array<std::shared_ptr<udsdx::SceneObject>, NumSkill> m_skillBackgrounds;
	std::array<std::shared_ptr<udsdx::SceneObject>, NumSkill> m_skillFills;
	std::array<float, NumSkill> m_skillFractions;
	std::array<bool, NumSkill> m_skillReady;

	float m_healthFillWidthCache = 0.0f;

	int m_maxHealth = 100;
	int m_currentHealth = 1;

	udsdx::GUIImage* m_healthFillRenderer;
};