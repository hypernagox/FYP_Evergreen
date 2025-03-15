#pragma once
#include "pch.h"

class Item;

class QuickSlot
{
public:
	Item* GetSlotItem(const uint32_t index)const {
		if (MAX_QUICK_SLOT <= index)return nullptr;
		return m_slotItems[index];
	}
	void SetSlotItem(Item* const item, const uint32_t index) {
		if (MAX_QUICK_SLOT <= index || m_slotItems[index])return;
		m_slotItems[index] = item;
	}
public:
	bool UseSlotItem(const uint32_t index)noexcept;
private:
	Item* m_slotItems[MAX_QUICK_SLOT] = { nullptr };
};

