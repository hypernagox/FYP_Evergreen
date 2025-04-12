#include "pch.h"
#include "Inventory.h"
#include "DropItem.h"
#include "Item.h"
#include "QuickSlot.h"

Item* Inventory::FindItem(const int8_t item_type) const noexcept
{
	for (int i = 0; i < NUM_OF_MAX_INVENTORY_ITEM; ++i) {
		if (m_items[i] && item_type == m_items[i]->m_itemDetailType)return m_items[i];
	}
	return nullptr;
}

int8_t Inventory::FindItemIndex(const int8_t item_type) const noexcept
{
	for (int i = 0; i < NUM_OF_MAX_INVENTORY_ITEM; ++i) {
		if (m_items[i] && item_type == m_items[i]->m_itemDetailType)return i;
	}
	return -1;
}

Inventory::~Inventory()
{
	for (int i = 0; i < NUM_OF_MAX_INVENTORY_ITEM; ++i) {
		if (m_items[i])NagiocpX::xdelete<Item>(m_items[i]);
	}
}

bool Inventory::CraftItem(const ItemRecipeData& recipe_info) noexcept
{
	bool flag = true;
	XVector<std::pair<Item*, int>> item_and_number;
	item_and_number.reserve(recipe_info.itemElements.size());
	for (const auto& item_info : recipe_info.itemElements)
	{
		if (const auto item = FindItem(item_info.itemID))
		{
			if (item->m_numOfItemStack >= item_info.numOfRequire)
			{
				item_and_number.emplace_back(item, item_info.numOfRequire);
			}
			else
			{
				flag = false;
				break;
			}
		}
		else
		{
			flag = false;
			break;
		}
	}
	if (flag)
	{
		for (const auto [item, number] : item_and_number)
		{
			item->m_numOfItemStack -= number;
			std::cout << (int)item->m_numOfItemStack << std::endl;
		}
	}
	return flag;
}

void Inventory::DecItemStack(const int8_t item_type, const int cnt) noexcept
{
	if (const auto item = FindItem(item_type))
	{
		item->m_numOfItemStack -= cnt;
		if (0 >= item->m_numOfItemStack)
		{
			const auto idx = FindItemIndex(item_type);
			if (-1 == idx)return;
			//NagiocpX::xdelete<Item>(m_items[idx]);
			//m_items[idx] = nullptr;
		}
	}
}

Item* Inventory::AddDropItem(const DropItem* const drop_item_info) noexcept
{
	const auto item_detail_type = drop_item_info->GetDropItemDetailInfo();
	const auto item_stack_size = drop_item_info->GetNumOfItemStack();
	if (const auto exist_item = FindItem(item_detail_type))
	{
		if (exist_item->m_numOfItemStack < 0) {
			NagiocpX::PrintLogEndl("Invaild Item Stack Size !");
		}
		exist_item->m_numOfItemStack += item_stack_size;
		return exist_item;
	}
	else
	{
		// TODO: 아이템종류 별로...
		// 아예 생성자로 줘버리기
		const auto item = NagiocpX::xnew<Item>();
		item->m_itemDetailType = item_detail_type;
		item->m_numOfItemStack = item_stack_size;
		if (AddItem2Inventory(item))
			return item;
		NagiocpX::xdelete<Item>(item);
		return nullptr;
	}
}

bool Inventory::SetQuickSlotItem(const uint8_t item_id, const uint8_t quick_idx) noexcept
{
	if (const auto item = FindItem(item_id))
	{
		return m_quickSlot.SetSlotItem(item, quick_idx);
	}
	else
		return false;
}
