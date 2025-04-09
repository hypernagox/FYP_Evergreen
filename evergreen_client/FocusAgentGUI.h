#pragma once

#include "pch.h"

class FocusAgentGUI : public udsdx::GUIElement
{
public:
	FocusAgentGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void Render(udsdx::RenderParam& param) override {};
	void SetTryClickCallback(std::function<void()> callback) { m_tryClickCallback = callback; }
	void SetTryExitCallback(std::function<void()> callback) { m_tryExitCallback = callback; }

protected:
	void OnMousePress() override;

	bool m_focused = false;
	std::function<void()> m_tryClickCallback = nullptr;
	std::function<void()> m_tryExitCallback = nullptr;
};