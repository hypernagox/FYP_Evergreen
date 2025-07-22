#pragma once

#include "pch.h"

class PlayerTagGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnAttach() override;
	void OnDetach() override;
	void SetText(std::wstring_view text);

private:
	std::shared_ptr<udsdx::SceneObject> m_nameObject;
};