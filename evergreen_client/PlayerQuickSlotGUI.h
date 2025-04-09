#pragma once

#include "pch.h"

class PlayerQuickSlotGUI : public udsdx::Component
{
public:
	PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void UpdateSlotContents(const std::vector<int>& table, const std::vector<int>& tableInventory);

private:
	static constexpr int NUM_SLOTS = 3;

	std::shared_ptr<udsdx::SceneObject> m_slotBackground[NUM_SLOTS];
	std::shared_ptr<udsdx::SceneObject> m_slotContents[NUM_SLOTS];
	std::shared_ptr<udsdx::SceneObject> m_slotText[NUM_SLOTS];
};