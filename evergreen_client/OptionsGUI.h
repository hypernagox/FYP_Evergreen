#pragma once

#include "pch.h"

class OptionsGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnActive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void OnButtonClick(int index);

private:
	std::shared_ptr<udsdx::SceneObject> m_background;
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::array<std::shared_ptr<udsdx::SceneObject>, 4> m_optionButtons;
	std::array<std::shared_ptr<udsdx::SceneObject>, 4> m_optionStatusTexts;
	std::array<bool, 4> m_optionStates;
};