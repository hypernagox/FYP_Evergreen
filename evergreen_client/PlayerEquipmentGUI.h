#pragma once

#include "pch.h"

class AuthenticPlayer;

class PlayerEquipmentGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnActive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void UpdateSlotContents(AuthenticPlayer* target, int index, int id);

private:
	int itemIDCache[2] = { -1, -1 };
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_slotContents[2];
	std::shared_ptr<udsdx::SceneObject> m_slotText[2];
	std::shared_ptr<udsdx::SceneObject> m_statText;
};