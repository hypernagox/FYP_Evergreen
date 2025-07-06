#pragma once

#include "pch.h"

class AuthenticPlayer;

class PlayerEquipmentGUI : public udsdx::Component
{
public:
	PlayerEquipmentGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void UpdateSlotContents(AuthenticPlayer* target, int index, int id);
	void SelectEquipmentSlot(AuthenticPlayer* target, int index);

private:
	int itemIDCache[2] = { -1, -1 };
	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_slotContents[2];
	std::shared_ptr<udsdx::SceneObject> m_slotText[2];
	std::shared_ptr<udsdx::SceneObject> m_statText;
};