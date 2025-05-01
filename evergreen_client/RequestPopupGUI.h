#pragma once

#include "pch.h"

class RequestPopupGUI : public udsdx::Component
{
public:
	RequestPopupGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void ShowPopup(std::wstring_view title, std::wstring_view contents, const std::function<void()>& onAccept, const std::function<void()>& onCancel);

private:
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_titleText;
	std::shared_ptr<udsdx::SceneObject> m_text;
	std::shared_ptr<udsdx::SceneObject> m_acceptButton;
	std::shared_ptr<udsdx::SceneObject> m_cancelButton;

	std::function<void()> m_onAccept = nullptr;
	std::function<void()> m_onCancel = nullptr;
};

