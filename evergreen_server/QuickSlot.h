#pragma once
#include "pch.h"

class Item;
class ContentsEntity;

class QuickSlot
{
public:
	Item* GetSlotItem(const uint32_t index)const {
		if (MAX_QUICK_SLOT <= index)return nullptr;
		return m_slotItems[index];
	}
	bool SetSlotItem(Item* const item, const uint32_t index) {
		if (MAX_QUICK_SLOT <= index)return false;
		m_slotItems[index] = item;
		return true;
	}
public:
	int8_t UseSlotItem(ContentsEntity* const owner,
		const uint32_t index)noexcept;
private:
	Item* m_slotItems[MAX_QUICK_SLOT] = { nullptr };
};

