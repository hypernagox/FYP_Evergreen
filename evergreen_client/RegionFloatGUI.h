#pragma once

#include "pch.h"

class RegionFloatGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void Float(std::wstring_view content);

private:
	std::shared_ptr<udsdx::SceneObject> m_backgroundObject;
	std::shared_ptr<udsdx::SceneObject> m_textObject;
	float m_elapsedTime = 0.0f;
};