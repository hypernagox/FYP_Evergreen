#pragma once

#include "pch.h"

class PlayerQuickSlotGUI : public udsdx::Component
{
public:
	PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetSlotContents(int slotIndex, uint8_t item_id);

private:
	static constexpr int NUM_SLOTS = 3;

	std::shared_ptr<udsdx::SceneObject> m_slotBackground[NUM_SLOTS];
	std::shared_ptr<udsdx::SceneObject> m_slotContents[NUM_SLOTS];
};