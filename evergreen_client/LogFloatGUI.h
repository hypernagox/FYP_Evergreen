#pragma once

#include "pch.h"

class LogFloatGUI : public udsdx::Component
{
public:
	LogFloatGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void AddText(const std::wstring& text);

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_floatText;

	float m_elapsedTime = 0.0f;
};

