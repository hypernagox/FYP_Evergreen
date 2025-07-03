#pragma once

#include "pch.h"

class TransitionOverlayGUI : public udsdx::Component
{
public:
	TransitionOverlayGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void BeginFadeOut();
	void AppendTransition(std::function<void()> callback, std::wstring_view message);

private:
	constexpr static float TransitionDuration = 0.5f;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_messageText;
	udsdx::GUIImage* m_backgroundRenderer = nullptr;

	std::vector<std::function<void()>> m_transitionCallbacks;
	float m_transitionDurationRemain = 0.0f;
};