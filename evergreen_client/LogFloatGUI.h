#pragma once

#include "pch.h"

class LogFloatGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void AddText(const std::wstring& text);
	bool GetAlwaysVisible() const { return m_alwaysVisible; }
	void SetAlwaysVisible(bool alwaysVisible) { m_alwaysVisible = alwaysVisible; }

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_floatText;

	float m_elapsedTime = 0.0f;
	bool m_alwaysVisible = false;
};

