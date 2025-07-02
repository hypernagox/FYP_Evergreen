#pragma once

#include "pch.h"

class FocusAgentGUI : public udsdx::GUIElement
{
public:
	FocusAgentGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Render(udsdx::RenderParam& param) override {};
	void SetTryClickCallback(std::function<void()> callback) { m_tryClickCallback = callback; }

protected:
	void OnMousePress() override;
	std::function<void()> m_tryClickCallback = nullptr;
};