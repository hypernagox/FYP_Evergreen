#pragma once

#include "pch.h"

class PlayerQuickSlotGUI : public udsdx::Component
{
public:
	PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void UpdateSlotContents(const std::vector<int>& table, const std::vector<int>& tableInventory);

private:
	std::shared_ptr<udsdx::SceneObject> m_slotBackground[MAX_QUICK_SLOT];
	std::shared_ptr<udsdx::SceneObject> m_slotContents[MAX_QUICK_SLOT];
	std::shared_ptr<udsdx::SceneObject> m_slotText[MAX_QUICK_SLOT];
};