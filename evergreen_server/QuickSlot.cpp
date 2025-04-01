#include "pch.h"
#include "QuickSlot.h"
#include "Item.h"
#include "Inventory.h"

bool QuickSlot::UseSlotItem(ContentsEntity* const owner,
	const uint32_t index) noexcept
{
	if (const auto item = GetSlotItem(index))
	{
		owner->GetComp<Inventory>()->DecItemStack(item->m_itemDetailType, 1);
		const bool res = item->UseItem(owner);
		NagiocpX::PrintLogEndl(std::format("남은 개수: {}", item->m_numOfItemStack));
		if (0 == item->m_numOfItemStack)
		{
			NagiocpX::PrintLogEndl("다씀");
			m_slotItems[index] = nullptr;
		}
		return res;
	}
	else return false;
}
