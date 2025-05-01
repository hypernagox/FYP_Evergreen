#pragma once

#include "pch.h"

class AuthenticPlayer;

class PlayerInventoryGUI : public udsdx::Component
{
public:
	PlayerInventoryGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void UpdateSlotContents(AuthenticPlayer* target, const std::vector<int>& table);

private:
	static constexpr int NUM_ROWS = 5;
	static constexpr int NUM_COLUMNS = 4;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_slotBackground[NUM_ROWS * NUM_COLUMNS];
	std::shared_ptr<udsdx::SceneObject> m_slotContents[NUM_ROWS * NUM_COLUMNS];
	std::shared_ptr<udsdx::SceneObject> m_slotText[NUM_ROWS * NUM_COLUMNS];
	std::shared_ptr<udsdx::SceneObject> m_coinText;
};