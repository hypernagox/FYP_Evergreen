#pragma once

#include "pch.h"

class InteractionFloatGUI : public udsdx::Component
{
public:
	InteractionFloatGUI(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetTargetPosition(const Vector3& targetPos) { m_targetPos = targetPos; }
	void SetText(const std::wstring& text);

private:
	Vector3 m_targetPos = Vector3::Zero;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_interactionText;
	std::shared_ptr<udsdx::SceneObject> m_interactionIcon;
};