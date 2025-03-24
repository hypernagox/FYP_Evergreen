#pragma once

#include "pch.h"

class PlayerStatusGUI : public udsdx::Component
{
public:
	PlayerStatusGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void SetMaxHealth(int value);
	void SetCurrentHealth(int value);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

private:
	std::shared_ptr<udsdx::SceneObject> m_healthBackground;
	std::shared_ptr<udsdx::SceneObject> m_healthFill;
	std::shared_ptr<udsdx::SceneObject> m_textObj;
	udsdx::GUIText* m_textRenderer;

	float m_healthFillWidthCache = 0.0f;

	int m_maxHealth = 100;
	int m_currentHealth = 1;

	udsdx::GUIImage* m_healthFillRenderer;
};