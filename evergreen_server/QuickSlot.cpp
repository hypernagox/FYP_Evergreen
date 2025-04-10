#include "pch.h"
#include "QuickSlot.h"
#include "Item.h"
#include "Inventory.h"

int8_t QuickSlot::UseSlotItem(ContentsEntity* const owner,
	const uint32_t index) noexcept
{
	if (const auto item = GetSlotItem(index))
	{
		if (0 == item->m_numOfItemStack)
		{
			return -1;
		}
		owner->GetComp<Inventory>()->DecItemStack(item->m_itemDetailType, 1);
		const bool res = item->UseItem(owner);
		const auto item_id = item->m_itemDetailType;
		NagiocpX::PrintLogEndl(std::format("남은 개수: {}", item->m_numOfItemStack));
		if (0 == item->m_numOfItemStack)
		{
			NagiocpX::PrintLogEndl("다씀");
		}
		return item_id;
	}
	else return -1;
}
