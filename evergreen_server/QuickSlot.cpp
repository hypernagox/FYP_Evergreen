#include "pch.h"
#include "QuickSlot.h"
#include "Item.h"

bool QuickSlot::UseSlotItem(const uint32_t index) noexcept
{
	if (const auto item = GetSlotItem(index))return item->UseItem();
	else return false;
}
