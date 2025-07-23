#pragma once

#include "pch.h"

class FocusAgentGUI : public udsdx::GUIElement
{
public:
	void Render(udsdx::RenderParam& param) override {};
	void SetTryClickCallback(std::function<void(int)> callback) { m_tryClickCallback = callback; }

protected:
	void OnMousePress(int mouse) override;
	std::function<void(int)> m_tryClickCallback = nullptr;
};