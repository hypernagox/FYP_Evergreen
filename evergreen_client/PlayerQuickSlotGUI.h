#pragma once

#include "pch.h"

class PlayerQuickSlotGUI : public udsdx::Component
{
public:
	PlayerQuickSlotGUI(const std::shared_ptr<udsdx::SceneObject>& object);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	// 인벤토리의 정보나 퀵슬롯의 정보가 수정되었을 경우 호출하는 함수
	void UpdateSlotContents(const std::vector<int>& tableQuickSlot, const std::vector<int>& tableInventory);

private:
	std::shared_ptr<udsdx::SceneObject> m_slotBackground[MAX_QUICK_SLOT];
	std::shared_ptr<udsdx::SceneObject> m_slotContents[MAX_QUICK_SLOT];
	std::shared_ptr<udsdx::SceneObject> m_slotText[MAX_QUICK_SLOT];
};